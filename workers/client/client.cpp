#include <cstdlib> // srand
#include <ctime> // time

#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

#include "canvas.h"
#include "chunk.h"
#include "chunk_layer.h"
#include "configuration.h"
#include "connect.h"
#include "drawable.h"
#include "interest.h"
#include "light.h"
#include "material.h"
#include "metadata.h"
#include "model.h"
#include "position.h"
#include "resource.h"
#include "sampler.h"
#include "texture.h"
#include "tile_sprite.h"
#include "tile_sprite_animation.h"
#include "tilemap.h"
#include "tilemap_colliders.h"
#include "tilemap_tiles.h"
#include "tileset.h"

extern "C" {
#include <shoveler/camera/perspective.h>
#include <shoveler/resources/image_png.h>
#include <shoveler/view/client.h>
#include <shoveler/view/position.h>
#include <shoveler/view/resources.h>
#include <shoveler/component.h>
#include <shoveler/constants.h>
#include <shoveler/game.h>
#include <shoveler/global.h>
#include <shoveler/input.h>
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
using shoveler::ChunkLayer;
using shoveler::Client;
using shoveler::ClientHeartbeat;
using shoveler::ClientInfo;
using shoveler::Drawable;
using shoveler::Light;
using shoveler::LightType;
using shoveler::Material;
using shoveler::Model;
using shoveler::Resource;
using shoveler::Sampler;
using shoveler::Texture;
using shoveler::Tilemap;
using shoveler::TilemapColliders;
using shoveler::TilemapTiles;
using shoveler::Tileset;
using shoveler::TileSprite;
using shoveler::TileSpriteAnimation;
using worker::Connection;
using worker::ConnectionParameters;
using worker::EntityId;
using worker::Option;
using worker::OutgoingCommandRequest;
using worker::RequestId;
using worker::Result;

using CreateClientEntity = Bootstrap::Commands::CreateClientEntity;
using ClientPing = Bootstrap::Commands::ClientPing;
using ClientSpawnCube = Bootstrap::Commands::ClientSpawnCube;
using DigHole = Bootstrap::Commands::DigHole;

struct ClientContext {
	Connection *connection;
	ShovelerGame *game;
	ClientConfiguration *clientConfiguration;
	EntityId bootstrapEntityId;
	EntityId clientEntityId;
	bool absoluteInterest;
	bool restrictController;
	bool viewDependenciesUpdated;
	double lastInterestUpdatePositionY;
	double edgeLength;
};

static const int64_t clientPingTimeoutMs = 1000;
static const char *const shovelerComponentViewTargetClientContext = "client_context";

static void updateGame(ShovelerGame *game, double dt);
static void updateAuthoritativeViewComponentFunction(ShovelerGame *game, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value);
static void clientPingTick(void *clientContextPointer);
static void mouseButtonEvent(ShovelerInput *input, int button, int action, int mods, void *clientContextPointer);
static void viewDependencyCallbackFunction(ShovelerView *view, const ShovelerViewQualifiedComponent *dependencySource, const ShovelerViewQualifiedComponent *dependencyTarget, bool added, void *contextPointer);
static void updateInterest(Connection& connection, EntityId clientEntityId, ShovelerView *view, bool absoluteInterest, ShovelerVector3 position, double edgeLength);
static void updateEdgeLength(ClientContext *context, ShovelerVector3 position);
static void keyHandler(ShovelerInput *input, int key, int scancode, int action, int mods, void *clientContextPointer);

int main(int argc, char **argv) {
	srand(time(NULL));

	ShovelerGameWindowSettings windowSettings;
	windowSettings.windowTitle = "ShovelerClient";
	windowSettings.fullscreen = false;
	windowSettings.vsync = true;
	windowSettings.samples = 4;
	windowSettings.windowedWidth = 640;
	windowSettings.windowedHeight = 480;

	shovelerLogInit("shoveler-spatialos/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);
	shovelerGlobalInit();

	if(argc != 1 && argc != 2 && argc != 4 && argc != 5) {
		shovelerLogError("Usage:\n\t%s\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>\n\t%s <locator hostname> <project name> <deployment name> <login token>", argv[0], argv[0], argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	const worker::ComponentRegistry& components = worker::Components<
		Bootstrap,
		Canvas,
		Chunk,
		ChunkLayer,
		Client,
		ClientHeartbeat,
		ClientInfo,
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
		Sampler,
		Texture,
		Tilemap,
		TilemapColliders,
		TilemapTiles,
		Tileset,
		TileSprite,
		TileSpriteAnimation>{};

	ConnectionParameters connectionParameters;
	connectionParameters.WorkerType = "ShovelerClient";
	connectionParameters.Network.ConnectionType = worker::NetworkConnectionType::kModularUdp;
	connectionParameters.Network.ModularUdp.SecurityType = worker::NetworkSecurityType::kDtls;
	worker::alpha::FlowControlParameters flowControlParameters;
	flowControlParameters.DownstreamWindowSizeBytes = 1 << 24;
	flowControlParameters.UpstreamWindowSizeBytes = 1 << 24;
	connectionParameters.Network.ModularUdp.FlowControl = {flowControlParameters};

	worker::Option<Connection> connectionOption = connect(argc, argv, connectionParameters, components);
	if(!connectionOption || !connectionOption->IsConnected()) {
		shovelerLogError("Failed to connect to SpatialOS deployment.");
		return EXIT_FAILURE;
	}
	Connection& connection = *connectionOption;
	shovelerLogInfo("Connected to SpatialOS deployment!");

	ClientConfiguration clientConfiguration = getClientConfiguration(connection);

	ShovelerGameCameraSettings cameraSettings;
	cameraSettings.frame = clientConfiguration.controllerSettings.frame;
	cameraSettings.projection.fieldOfViewY = 2.0f * SHOVELER_PI * 50.0f / 360.0f;
	cameraSettings.projection.aspectRatio = (float) windowSettings.windowedWidth / windowSettings.windowedHeight;
	cameraSettings.projection.nearClippingPlane = 0.01;
	cameraSettings.projection.farClippingPlane = 1000;

	ShovelerGame *game = shovelerGameCreate(updateGame, updateAuthoritativeViewComponentFunction, &windowSettings, &cameraSettings, &clientConfiguration.controllerSettings);
	if(game == NULL) {
		return EXIT_FAILURE;
	}
	ShovelerView *view = game->view;

	ClientContext context;
	context.connection = &connection;
	context.game = game;
	context.clientConfiguration = &clientConfiguration;
	context.bootstrapEntityId = 0;
	context.clientEntityId = 0;
	context.absoluteInterest = false;
	context.restrictController = true;
	context.viewDependenciesUpdated = false;
	context.lastInterestUpdatePositionY = 0.0f;
	context.edgeLength = 20.5f;

	shovelerViewSetTarget(view, shovelerComponentViewTargetClientContext, &context);

	shovelerInputAddKeyCallback(game->input, keyHandler, &context);

	game->controller->lockMoveX = clientConfiguration.controllerLockMoveX;
	game->controller->lockMoveY = clientConfiguration.controllerLockMoveY;
	game->controller->lockMoveZ = clientConfiguration.controllerLockMoveZ;
	game->controller->lockTiltX = clientConfiguration.controllerLockTiltX;
	game->controller->lockTiltY = clientConfiguration.controllerLockTiltY;

	worker::Dispatcher dispatcher{components};
	bool disconnected = false;
	ShovelerExecutorCallback *clientPingTickCallback = NULL;
	bool clientInterestAuthoritative = false;

	ShovelerInputMouseButtonCallback *mouseButtonCallback = shovelerInputAddMouseButtonCallback(game->input, mouseButtonEvent, &context);

	ShovelerResources *resources = shovelerResourcesCreate(/* TODO: on demand resource loading */ NULL, NULL);
	shovelerResourcesImagePngRegister(resources);

	shovelerViewAddDependencyCallback(view, viewDependencyCallbackFunction, &context);
	shovelerViewSetResources(view, resources);

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
		shovelerLogTrace("Adding entity %lld.", op.EntityId);
		shovelerViewAddEntity(view, op.EntityId);
	});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op) {
		shovelerLogTrace("Removing entity %lld.", op.EntityId);
		shovelerViewRemoveEntity(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Client>([&](const worker::AddComponentOp<Client>& op) {
		shovelerLogTrace("Adding client to entity %lld.", op.EntityId);
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewClientConfiguration clientComponentConfiguration;
		clientComponentConfiguration.positionEntityId = op.EntityId;

		if(clientConfiguration.hidePlayerClientEntityModel) {
			clientComponentConfiguration.modelEntityId = op.EntityId;
		} else {
			clientComponentConfiguration.modelEntityId = {};
		}

		shovelerViewEntityAddClient(entity, &clientComponentConfiguration);
	});

	dispatcher.OnAuthorityChange<Client>([&](const worker::AuthorityChangeOp& op) {
		shovelerLogInfo("Changing client authority for entity %lld to %d.", op.EntityId, op.Authority);
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		if(op.Authority == worker::Authority::kAuthoritative) {
			shovelerViewEntityDelegate(entity, shovelerComponentTypeIdClient);
			shovelerComponentActivate(shovelerViewEntityGetComponent(entity, shovelerComponentTypeIdClient));
			clientPingTickCallback = shovelerExecutorSchedulePeriodic(game->updateExecutor, 0, clientPingTimeoutMs, clientPingTick, &context);
			context.clientEntityId = op.EntityId;
		} else if(op.Authority == worker::Authority::kNotAuthoritative) {
			shovelerViewEntityUndelegate(entity, shovelerComponentTypeIdClient);
			shovelerExecutorRemoveCallback(game->updateExecutor, clientPingTickCallback);
			context.clientEntityId = 0;
		}
	});

	dispatcher.OnRemoveComponent<Client>([&](const worker::RemoveComponentOp& op) {
		shovelerLogTrace("Removing client from entity %lld.", op.EntityId);
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveClient(entity);
	});

	dispatcher.OnAuthorityChange<Interest>([&](const worker::AuthorityChangeOp& op) {
		if(op.Authority == worker::Authority::kAuthoritative) {
			shovelerLogInfo("Received authority over interest component of client entity %lld.", op.EntityId);
			clientInterestAuthoritative = true;
			context.viewDependenciesUpdated = true;
		} else {
			clientInterestAuthoritative = false;
		}
	});

	worker::query::EntityQuery bootstrapEntityQuery{worker::query::ComponentConstraint{Bootstrap::ComponentId}, worker::query::SnapshotResultType{{{Bootstrap::ComponentId}}}};
	worker::RequestId<worker::EntityQueryRequest> bootstrapQueryRequestId = connection.SendEntityQueryRequest(bootstrapEntityQuery, {});

	dispatcher.OnEntityQueryResponse([&](const worker::EntityQueryResponseOp& op) {
		shovelerLogInfo("Received entity query response for request %lld with status code %d.", op.RequestId.Id, op.StatusCode);
		if(op.RequestId == bootstrapQueryRequestId && !op.Result.empty()) {
			context.bootstrapEntityId = op.Result.begin()->first;
			shovelerLogInfo("Received bootstrap query response containing entity %lld.", context.bootstrapEntityId);

			Result<RequestId<OutgoingCommandRequest<CreateClientEntity>>> createClientEntityRequestId = connection.SendCommandRequest<CreateClientEntity>(context.bootstrapEntityId, {}, {});
			if(!createClientEntityRequestId) {
				shovelerLogError("Failed to send create client entity request: %s", createClientEntityRequestId.GetErrorMessage().c_str());
				return;
			}

			shovelerLogInfo("Sent create client entity request with id %lld.", createClientEntityRequestId->Id);
		}
	});

	dispatcher.OnCommandResponse<CreateClientEntity>([&](const worker::CommandResponseOp<CreateClientEntity>& op) {
		shovelerLogInfo("Received create client entity command response %lld with status code %d.", op.RequestId.Id, op.StatusCode);
	});

	registerPositionCallbacks(connection, dispatcher, view, clientConfiguration.positionMappingX, clientConfiguration.positionMappingY, clientConfiguration.positionMappingZ);
	registerMetadataCallbacks(dispatcher, view);
	registerResourceCallbacks(dispatcher, view);
	registerSamplerCallbacks(dispatcher, view);
	registerTextureCallbacks(dispatcher, view);
	registerTilesetCallbacks(dispatcher, view);
	registerTilemapCollidersCallbacks(dispatcher, view);
	registerTilemapTilesCallbacks(dispatcher, view);
	registerTilemapCallbacks(dispatcher, view);
	registerTileSpriteCallbacks(dispatcher, view);
	registerTileSpriteAnimationCallbacks(dispatcher, view);
	registerCanvasCallbacks(dispatcher, view);
	registerChunkCallbacks(dispatcher, view);
	registerChunkLayerCallbacks(dispatcher, view);
	registerDrawableCallbacks(dispatcher, view);
	registerMaterialCallbacks(dispatcher, view);
	registerModelCallbacks(dispatcher, view);
	registerLightCallbacks(dispatcher, view);

	while(shovelerGameIsRunning(game) && !disconnected) {
		context.viewDependenciesUpdated = false;

		dispatcher.Process(connection.GetOpList(0));
		shovelerGameRenderFrame(game);

		ShovelerVector3 position = getEntitySpatialOsPosition(view, clientConfiguration.positionMappingX, clientConfiguration.positionMappingY, clientConfiguration.positionMappingZ, context.clientEntityId);
		updateEdgeLength(&context, position);

		if(clientInterestAuthoritative && context.viewDependenciesUpdated) {
			updateInterest(connection, context.clientEntityId, view, context.absoluteInterest, position, context.edgeLength);
		}
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	shovelerInputRemoveMouseButtonCallback(game->input, mouseButtonCallback);

	shovelerGameFree(game);
	shovelerResourcesFree(resources);
	shovelerGlobalUninit();
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}

static void updateGame(ShovelerGame *game, double dt) {
	shovelerCameraUpdateView(game->camera);
}

static void updateAuthoritativeViewComponentFunction(ShovelerGame *game, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value)
{
	ClientContext *context = (ClientContext *) shovelerViewGetTarget(game->view, shovelerComponentViewTargetClientContext);

	if(component->type->id == shovelerComponentTypeIdPosition) {
		requestPositionUpdate(*context->connection, game->view, context->clientConfiguration->positionMappingX, context->clientConfiguration->positionMappingY, context->clientConfiguration->positionMappingZ, component, configurationOption, value);
	}
}

static void clientPingTick(void *clientContextPointer) {
	ClientContext *context = (ClientContext *) clientContextPointer;
	context->connection->SendCommandRequest<ClientPing>(context->bootstrapEntityId, {context->clientEntityId}, {});
}

static void mouseButtonEvent(ShovelerInput *input, int button, int action, int mods, void *clientContextPointer) {
	ClientContext *context = (ClientContext *) clientContextPointer;

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		const Option<std::string>& flagOption = context->connection->GetWorkerFlag("game_type");
		if(flagOption && *flagOption == "tiles") {
			shovelerLogInfo("Sending dig hole command...");
			context->connection->SendCommandRequest<DigHole>(context->bootstrapEntityId, {context->clientEntityId}, {});
		} else {
			shovelerLogInfo("Sending client cube spawn command...");
			ShovelerCameraPerspective *perspectiveCamera = (ShovelerCameraPerspective *) context->game->camera->data;
			context->connection->SendCommandRequest<ClientSpawnCube>(context->bootstrapEntityId, {context->clientEntityId, {-perspectiveCamera->direction.values[0], perspectiveCamera->direction.values[1], perspectiveCamera->direction.values[2]}, {0.0f, 0.0f, 0.0f}}, {});
		}
	}
}

static void viewDependencyCallbackFunction(ShovelerView *view, const ShovelerViewQualifiedComponent *dependencySource, const ShovelerViewQualifiedComponent *dependencyTarget, bool added, void *contextPointer) {
	ClientContext *context = (ClientContext *) contextPointer;
	context->viewDependenciesUpdated = true;
}

static void updateInterest(Connection& connection, EntityId clientEntityId, ShovelerView *view, bool absoluteInterest, ShovelerVector3 position, double edgeLength) {
	ComponentInterest interest = computeViewInterest(view, absoluteInterest, position, edgeLength);

	Interest::Update interestUpdate;
	interestUpdate.set_component_interest({{Client::ComponentId, interest}});
	connection.SendComponentUpdate<Interest>(clientEntityId, interestUpdate);

	shovelerLogInfo("Sent interest update with %zu queries.", interest.queries().size());
}

static void updateEdgeLength(ClientContext *context, ShovelerVector3 position) {
	if(context->absoluteInterest) {
		return;
	}

	double positionChangeY = fabs(position.values[1] - context->lastInterestUpdatePositionY);
	if(fabs(positionChangeY) < 0.5f) {
		return;
	}

	context->edgeLength = 20.5f * position.values[1] / 5.0f;
	if(context->edgeLength < 20.5f) {
		context->edgeLength = 20.5f;
	}

	context->lastInterestUpdatePositionY = position.values[1];
	context->viewDependenciesUpdated = true;
}

static void keyHandler(ShovelerInput *input, int key, int scancode, int action, int mods, void *clientContextPointer) {
	ClientContext *context = (ClientContext *) clientContextPointer;

	if(key == GLFW_KEY_F7 && action == GLFW_PRESS) {
		shovelerLogInfo("F7 key pressed, changing to %s interest.", context->absoluteInterest ? "relative" : "absolute");
		context->absoluteInterest = !context->absoluteInterest;

		ShovelerVector3 position = getEntitySpatialOsPosition(context->game->view, context->clientConfiguration->positionMappingX, context->clientConfiguration->positionMappingY, context->clientConfiguration->positionMappingZ, context->clientEntityId);
		updateEdgeLength(context, position);
		updateInterest(*context->connection, context->clientEntityId, context->game->view, context->absoluteInterest, position, context->edgeLength);
	}

	if(key == GLFW_KEY_F8 && action == GLFW_PRESS) {
		shovelerLogInfo("F8 key pressed, %s controller.", context->restrictController ? "unrestricting" : "restricting");
		context->restrictController = !context->restrictController;
		if(context->restrictController) {
			context->game->controller->lockMoveX = context->clientConfiguration->controllerLockMoveX;
			context->game->controller->lockMoveY = context->clientConfiguration->controllerLockMoveY;
			context->game->controller->lockMoveZ = context->clientConfiguration->controllerLockMoveZ;
			context->game->controller->lockTiltX = context->clientConfiguration->controllerLockTiltX;
			context->game->controller->lockTiltY = context->clientConfiguration->controllerLockTiltY;
		} else {
			context->game->controller->lockMoveX = false;
			context->game->controller->lockMoveY = false;
			context->game->controller->lockMoveZ = false;
			context->game->controller->lockTiltX = false;
			context->game->controller->lockTiltY = false;
		}
	}
}
