#include <cstdlib> // srand
#include <ctime> // time

#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

#include "canvas.h"
#include "chunk.h"
#include "connect.h"
#include "drawable.h"
#include "interest.h"
#include "light.h"
#include "material.h"
#include "metadata.h"
#include "model.h"
#include "position.h"
#include "resource.h"
#include "texture.h"
#include "tile_sprite.h"
#include "tile_sprite_animation.h"
#include "tilemap.h"
#include "tilemap_tiles.h"
#include "tileset.h"

extern "C" {
#include <shoveler/camera/perspective.h>
#include <shoveler/resources/image_png.h>
#include <shoveler/view/client.h>
#include <shoveler/view/resources.h>
#include <shoveler/constants.h>
#include <shoveler/game.h>
#include <shoveler/global.h>
#include <shoveler/log.h>
#include <shoveler/resources.h>
#include <shoveler/scene.h>
#include <shoveler/view.h>
}

using improbable::ComponentInterest;
using improbable::EntityAcl;
using improbable::Interest;
using improbable::Metadata;
using improbable::Persistence;
using improbable::Position;
using shoveler::Bootstrap;
using shoveler::Canvas;
using shoveler::Chunk;
using shoveler::Client;
using shoveler::Drawable;
using shoveler::Light;
using shoveler::LightType;
using shoveler::Material;
using shoveler::Model;
using shoveler::Resource;
using shoveler::Texture;
using shoveler::Tilemap;
using shoveler::TilemapTiles;
using shoveler::Tileset;
using shoveler::TileSprite;
using shoveler::TileSpriteAnimation;
using worker::Connection;
using worker::ConnectionParameters;
using worker::EntityId;

using CreateClientEntity = Bootstrap::Commands::CreateClientEntity;
using ClientPing = Bootstrap::Commands::ClientPing;
using ClientSpawnCube = Bootstrap::Commands::ClientSpawnCube;

struct ClientPingTickContext {
	Connection *connection;
	EntityId bootstrapEntityId;
	EntityId clientEntityId;
};

struct MouseButtonEventContext {
	Connection *connection;
	ShovelerCamera *camera;
	EntityId *bootstrapEntityId;
	EntityId *clientEntityId;
};

struct ViewDependencyCallbackContext {
	bool viewDependenciesUpdated;
};

static const int64_t clientPingTimeoutMs = 1000;

static void updateGame(ShovelerGame *game, double dt);
static void clientPingTick(void *clientPingTickContextPointer);
static void mouseButtonEvent(ShovelerInput *input, int button, int action, int mods, void *mouseButtonEventContextPointer);
static void viewDependencyCallbackFunction(ShovelerView *view, const ShovelerViewQualifiedComponent *dependencySource, const ShovelerViewQualifiedComponent *dependencyTarget, bool added, void *contextPointer);
static void updateInterest(Connection& connection, EntityId clientEntityId, ShovelerView *view);

int main(int argc, char **argv) {
	srand(time(NULL));

	ShovelerGameWindowSettings windowSettings;
	windowSettings.windowTitle = "ShovelerClient";
	windowSettings.fullscreen = false;
	windowSettings.vsync = true;
	windowSettings.samples = 4;
	windowSettings.windowedWidth = 640;
	windowSettings.windowedHeight = 480;

	ShovelerGameControllerSettings controllerSettings;
	controllerSettings.position = shovelerVector3(0, 0, -1);
	controllerSettings.direction = shovelerVector3(0, 0, 1);
	controllerSettings.up = shovelerVector3(0, 1, 0);
	controllerSettings.moveFactor = 2.0f;
	controllerSettings.tiltFactor = 0.0005f;

	float fov = 2.0f * SHOVELER_PI * 50.0f / 360.0f;
	float aspectRatio = (float) windowSettings.windowedWidth / windowSettings.windowedHeight;

	ShovelerCoordinateMapping positionMappingX = SHOVELER_COORDINATE_MAPPING_NEGATIVE_X;
	ShovelerCoordinateMapping positionMappingY = SHOVELER_COORDINATE_MAPPING_POSITIVE_Y;
	ShovelerCoordinateMapping positionMappingZ = SHOVELER_COORDINATE_MAPPING_POSITIVE_Z;

	shovelerLogInit("shoveler-spatialos/workers/cmake/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);
	shovelerGlobalInit();

	if (argc != 1 && argc != 2 && argc != 4) {
		shovelerLogError("Usage:\n\t%s\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>", argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	ShovelerCamera *camera = shovelerCameraPerspectiveCreate(controllerSettings.position, controllerSettings.direction, controllerSettings.up, fov, aspectRatio, 0.01, 1000);

	ShovelerGame *game = shovelerGameCreate(camera, updateGame, &windowSettings, &controllerSettings);
	if(game == NULL) {
		return EXIT_FAILURE;
	}
	ShovelerView *view = game->view;

	const worker::ComponentRegistry& components = worker::Components<
		Bootstrap,
		Canvas,
		Chunk,
		Client,
		Drawable,
		EntityAcl,
		Interest,
		Light,
		Metadata,
		Material,
		Model,
		Persistence,
		Position,
		Resource,
		Texture,
		Tilemap,
		TilemapTiles,
		Tileset,
		TileSprite,
        TileSpriteAnimation>{};

	ConnectionParameters connectionParameters;
	connectionParameters.WorkerType = "ShovelerClient";
	connectionParameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;

	worker::Option<Connection> connectionOption = connect(argc, argv, connectionParameters, components);
	if(!connectionOption || !connectionOption->IsConnected()) {
		return EXIT_FAILURE;
	}
	Connection& connection = *connectionOption;
	shovelerLogInfo("Connected to SpatialOS deployment!");

	worker::Dispatcher dispatcher{components};
	bool disconnected = false;
	EntityId bootstrapEntityId = 0;
	EntityId clientEntityId = 0;
	ClientPingTickContext clientPingTickContext;
	ViewDependencyCallbackContext viewDependencyCallbackContext{false};
	ShovelerExecutorCallback *clientPingTickCallback = NULL;
	bool clientInterestAuthoritative = false;

	MouseButtonEventContext mouseButtonEventContext{&connection, game->camera, &bootstrapEntityId, &clientEntityId};
	ShovelerInputMouseButtonCallback *mouseButtonCallback = shovelerInputAddMouseButtonCallback(game->input, mouseButtonEvent, &mouseButtonEventContext);

	shovelerCameraPerspectiveAttachController(camera, game->controller);

	ShovelerResources *resources = shovelerResourcesCreate(/* TODO: on demand resource loading */ NULL, NULL);
	shovelerResourcesImagePngRegister(resources);

	shovelerViewAddDependencyCallback(view, viewDependencyCallbackFunction, &viewDependencyCallbackContext);
	shovelerViewSetTarget(view, shovelerViewResourcesTargetName, resources);

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		shovelerLogError("Disconnected from SpatialOS: %s", op.Reason.c_str());
		disconnected = true;
	});

	dispatcher.OnMetrics([&](const worker::MetricsOp& op) {
		auto metrics = op.Metrics;
		connection.SendMetrics(metrics);
	});

	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		switch(op.Level) {
			case worker::LogLevel::kDebug:
				shovelerLogTrace(op.Message.c_str());
				break;
			case worker::LogLevel::kInfo:
				shovelerLogInfo(op.Message.c_str());
				break;
			case worker::LogLevel::kWarn:
				shovelerLogWarning(op.Message.c_str());
				break;
			case worker::LogLevel::kError:
				shovelerLogError(op.Message.c_str());
				break;
			case worker::LogLevel::kFatal:
				shovelerLogError(op.Message.c_str());
				std::terminate();
			default:
				shovelerLogWarning("SpatialOS SDK logged message with unknown log level %d: %s", op.Level, op.Message.c_str());
				break;
		}
	});

	dispatcher.OnAddEntity([&](const worker::AddEntityOp& op) {
		shovelerLogInfo("Adding entity %lld.", op.EntityId);
		shovelerViewAddEntity(view, op.EntityId);
	});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op) {
		shovelerLogInfo("Removing entity %lld.", op.EntityId);
		shovelerViewRemoveEntity(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Client>([&](const worker::AddComponentOp<Client>& op) {
		shovelerLogInfo("Adding client to entity %lld.", op.EntityId);
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewClientConfiguration clientConfiguration;
		clientConfiguration.positionEntityId = op.EntityId;
		clientConfiguration.modelEntityId = op.EntityId;
		clientConfiguration.disableModelVisibility = true;
		shovelerViewEntityAddClient(entity, &clientConfiguration);
	});

	dispatcher.OnAuthorityChange<Client>([&](const worker::AuthorityChangeOp& op) {
		shovelerLogInfo("Changing client authority for entity %lld to %d.", op.EntityId, op.Authority);
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		if (op.Authority == worker::Authority::kAuthoritative) {
			shovelerViewEntityDelegateClient(entity);
			clientPingTickContext = ClientPingTickContext{&connection, bootstrapEntityId, op.EntityId};
			clientPingTickCallback = shovelerExecutorSchedulePeriodic(game->updateExecutor, 0, clientPingTimeoutMs, clientPingTick, &clientPingTickContext);
			clientEntityId = op.EntityId;
		} else if (op.Authority == worker::Authority::kNotAuthoritative) {
			shovelerViewEntityUndelegateClient(entity);
			shovelerExecutorRemoveCallback(game->updateExecutor, clientPingTickCallback);
			clientEntityId = 0;
		}
	});

	dispatcher.OnRemoveComponent<Client>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing client from entity %lld.", op.EntityId);
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveClient(entity);
	});

	dispatcher.OnAuthorityChange<Interest>([&](const worker::AuthorityChangeOp& op) {
		if(op.EntityId == clientEntityId) {
			if(op.Authority == worker::Authority::kAuthoritative) {
				shovelerLogInfo("Received authority over interest component of client entity %lld.", op.EntityId);
				clientInterestAuthoritative = true;
				viewDependencyCallbackContext.viewDependenciesUpdated = true;
			} else {
				clientInterestAuthoritative = false;
			}
		}
	});

	worker::query::EntityQuery bootstrapEntityQuery{worker::query::ComponentConstraint{Bootstrap::ComponentId}, worker::query::SnapshotResultType{{{Bootstrap::ComponentId}}}};
	worker::RequestId<worker::EntityQueryRequest> bootstrapQueryRequestId = connection.SendEntityQueryRequest(bootstrapEntityQuery, {});

	dispatcher.OnEntityQueryResponse([&](const worker::EntityQueryResponseOp& op) {
		shovelerLogInfo("Received entity query response for request %u with status code %d.", op.RequestId.Id, op.StatusCode);
		if(op.RequestId == bootstrapQueryRequestId && op.Result.size() > 0) {
			bootstrapEntityId = op.Result.begin()->first;
			shovelerLogInfo("Received bootstrap query response containing entity %lld.", bootstrapEntityId);
			worker::RequestId<worker::OutgoingCommandRequest<CreateClientEntity>> createClientEntityRequestId = connection.SendCommandRequest<CreateClientEntity>(bootstrapEntityId, {}, {});
			shovelerLogInfo("Sent create client entity request with id %u.", createClientEntityRequestId.Id);
		}
	});

	dispatcher.OnCommandResponse<CreateClientEntity>([&](const worker::CommandResponseOp<CreateClientEntity>& op) {
		shovelerLogInfo("Received create client entity command response %u with status code %d.", op.RequestId.Id, op.StatusCode);
	});

	registerPositionCallbacks(connection, dispatcher, view, positionMappingX, positionMappingY, positionMappingZ);
	registerMetadataCallbacks(dispatcher, view);
	registerResourceCallbacks(dispatcher, view);
	registerTextureCallbacks(dispatcher, view);
	registerTilesetCallbacks(dispatcher, view);
	registerTilemapTilesCallbacks(dispatcher, view);
	registerTilemapCallbacks(dispatcher, view);
	registerTileSpriteCallbacks(dispatcher, view);
	registerTileSpriteAnimationCallbacks(dispatcher, view);
	registerCanvasCallbacks(dispatcher, view);
	registerChunkCallbacks(dispatcher, view);
	registerDrawableCallbacks(dispatcher, view);
	registerMaterialCallbacks(dispatcher, view);
	registerModelCallbacks(dispatcher, view);
	registerLightCallbacks(dispatcher, view);

	while(shovelerGameIsRunning(game) && !disconnected) {
		viewDependencyCallbackContext.viewDependenciesUpdated = false;

		dispatcher.Process(connection.GetOpList(0));
		shovelerGameRenderFrame(game);

		if(clientInterestAuthoritative && viewDependencyCallbackContext.viewDependenciesUpdated) {
			updateInterest(connection, clientEntityId, view);
		}
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	shovelerInputRemoveMouseButtonCallback(game->input, mouseButtonCallback);

	shovelerCameraPerspectiveDetachController(camera);
	shovelerGameFree(game);
	shovelerResourcesFree(resources);
	shovelerCameraFree(camera);
	shovelerGlobalUninit();
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}

static void updateGame(ShovelerGame *game, double dt)
{
	shovelerCameraUpdateView(game->camera);
}

static void clientPingTick(void *clientPingTickContextPointer)
{
	ClientPingTickContext *context = (ClientPingTickContext *) clientPingTickContextPointer;
	context->connection->SendCommandRequest<ClientPing>(context->bootstrapEntityId, {context->clientEntityId}, {});
}

static void mouseButtonEvent(ShovelerInput *input, int button, int action, int mods, void *mouseButtonEventContextPointer)
{
	MouseButtonEventContext *context = (MouseButtonEventContext *) mouseButtonEventContextPointer;

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		shovelerLogInfo("Sending client cube spawn command...");
		ShovelerCameraPerspective *perspectiveCamera = (ShovelerCameraPerspective *) context->camera->data;
		context->connection->SendCommandRequest<ClientSpawnCube>(*context->bootstrapEntityId, {*context->clientEntityId, {-perspectiveCamera->direction.values[0], perspectiveCamera->direction.values[1], perspectiveCamera->direction.values[2]}, {0.0f, 0.0f, 0.0f}}, {});
	}
}

static void viewDependencyCallbackFunction(ShovelerView *view, const ShovelerViewQualifiedComponent *dependencySource, const ShovelerViewQualifiedComponent *dependencyTarget, bool added, void *contextPointer)
{
	ViewDependencyCallbackContext *context = (ViewDependencyCallbackContext *) contextPointer;
	context->viewDependenciesUpdated = true;
}

static void updateInterest(Connection& connection, EntityId clientEntityId, ShovelerView *view)
{
	ComponentInterest interest = computeViewInterest(view);

	Interest::Update interestUpdate;
	interestUpdate.set_component_interest({{Client::ComponentId, interest}});
	connection.SendComponentUpdate<Interest>(clientEntityId, interestUpdate);

	shovelerLogInfo("Sent interest update with %zu queries.", interest.queries().size());
}
