#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <glib.h>

#include <shoveler/camera/perspective.h>
#include <shoveler/view/client.h>
#include <shoveler/view/controller.h>
#include <shoveler/view/drawables.h>
#include <shoveler/view/light.h>
#include <shoveler/view/model.h>
#include <shoveler/view/position.h>
#include <shoveler/view/scene.h>
#include <shoveler/constants.h>
#include <shoveler/controller.h>
#include <shoveler/controller.h>
#include <shoveler/game.h>
#include <shoveler/global.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
#include <shoveler/view.h>
}

using shoveler::Bootstrap;
using shoveler::Client;
using shoveler::Drawable;
using shoveler::DrawableType;
using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Model;
using shoveler::Light;
using shoveler::LightType;
using shoveler::PolygonMode;
using improbable::Coordinates;
using improbable::Position;

using CreateClientEntity = shoveler::Bootstrap::Commands::CreateClientEntity;
using ClientPing = shoveler::Bootstrap::Commands::ClientPing;
using ClientSpawnCube = shoveler::Bootstrap::Commands::ClientSpawnCube;

struct PositionUpdateRequestContext {
	worker::Connection *connection;
};

struct ClientPingTickContext {
	worker::Connection *connection;
	worker::EntityId bootstrapEntityId;
	worker::EntityId clientEntityId;
};

struct MouseButtonEventContext {
	worker::Connection *connection;
	ShovelerCamera *camera;
	worker::EntityId *bootstrapEntityId;
	worker::EntityId *clientEntityId;
};

static worker::Option<worker::Connection> connect(int argc, char **argv, const worker::ComponentRegistry& components);
static void updateGame(ShovelerGame *game, double dt);
static void requestPositionUpdate(ShovelerViewComponent *component, double x, double y, double z, void *positionUpdateRequestContextPointer);
static void clientPingTick(void *clientPingTickContextPointer);
static void mouseButtonEvent(ShovelerInput *input, int button, int action, int mods, void *mouseButtonEventContextPointer);
static ShovelerViewDrawableConfiguration createDrawableConfiguration(const Drawable& drawable);
static ShovelerViewMaterialConfiguration createMaterialConfiguration(const Material& material);
static GLuint getPolygonMode(PolygonMode polygonMode);

static ShovelerController *controller;

int main(int argc, char **argv) {
	const char *windowTitle = "ShovelerClient";
	bool fullscreen = false;
	bool vsync = true;
	int samples = 4;
	int width = 640;
	int height = 480;

	shovelerLogInit("shoveler-spatialos/workers/cmake/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);
	shovelerGlobalInit();

	if (argc != 2 && argc != 4) {
		shovelerLogError("Usage:\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>", argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	const worker::ComponentRegistry& components = worker::Components<
		shoveler::Bootstrap,
		shoveler::Client,
		shoveler::Light,
		shoveler::Model,
		improbable::EntityAcl,
		improbable::Metadata,
		improbable::Persistence,
		improbable::Position>{};

	worker::Option<worker::Connection> connectionOption = connect(argc, argv, components);
	if(!connectionOption || !connectionOption->IsConnected()) {
		return EXIT_FAILURE;
	}
	worker::Connection& connection = *connectionOption;
	shovelerLogInfo("Connected to SpatialOS deployment!");

	const int64_t clientPingTimeoutMs = 1000;
	bool disconnected = false;
	worker::Dispatcher dispatcher{components};
	worker::EntityId bootstrapEntityId = 0;
	worker::EntityId clientEntityId = 0;
	ClientPingTickContext clientPingTickContext;
	ShovelerExecutorCallback *clientPingTickCallback = NULL;

	ShovelerGame *game = shovelerGameCreate(windowTitle, width, height, samples, fullscreen, vsync);
	if(game == NULL) {
		return EXIT_FAILURE;
	}

	game->camera = shovelerCameraPerspectiveCreate(ShovelerVector3{0.0, 0.0, -1.0}, ShovelerVector3{0.0, 0.0, 1.0}, ShovelerVector3{0.0, 1.0, 0.0}, 2.0f * SHOVELER_PI * 50.0f / 360.0f, (float) width / height, 0.01, 1000);
	game->scene = shovelerSceneCreate();
	game->update = updateGame;

	MouseButtonEventContext mouseButtonEventContext{&connection, game->camera, &bootstrapEntityId, &clientEntityId};
	ShovelerInputMouseButtonCallback *mouseButtonCallback = shovelerInputAddMouseButtonCallback(game->input, mouseButtonEvent, &mouseButtonEventContext);

	controller = shovelerControllerCreate(game, ShovelerVector3{0.0, 0.0, -1.0}, ShovelerVector3{0.0, 0.0, 1.0}, ShovelerVector3{0.0, 1.0, 0.0}, 2.0f, 0.0005f);
	shovelerCameraPerspectiveAttachController(game->camera, controller);

	ShovelerView *view = shovelerViewCreate();
	shovelerViewSetTarget(view, "controller", controller);
	shovelerViewSetTarget(view, "scene", game->scene);

	ShovelerViewDrawables *drawables = shovelerViewDrawablesCreate(view);

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
		shovelerViewAddEntityClient(view, op.EntityId);
	});

	dispatcher.OnAuthorityChange<Client>([&](const worker::AuthorityChangeOp& op) {
		shovelerLogInfo("Changing client authority for entity %lld to %d.", op.EntityId, op.Authority);
		if (op.Authority == worker::Authority::kAuthoritative) {
			shovelerViewDelegateClient(view, op.EntityId);
			clientPingTickContext = ClientPingTickContext{&connection, bootstrapEntityId, op.EntityId};
			clientPingTickCallback = shovelerExecutorSchedulePeriodic(game->updateExecutor, 0, clientPingTimeoutMs, clientPingTick, &clientPingTickContext);
			clientEntityId = op.EntityId;
		} else if (op.Authority == worker::Authority::kNotAuthoritative) {
			shovelerViewUndelegateClient(view, op.EntityId);
			shovelerExecutorRemoveCallback(game->updateExecutor, clientPingTickCallback);
			clientEntityId = 0;
		}
	});

	dispatcher.OnRemoveComponent<Client>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing client from entity %lld.", op.EntityId);
		shovelerViewRemoveEntityClient(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Position>([&](const worker::AddComponentOp<Position>& op) {
		shovelerLogInfo("Adding position to entity %lld.", op.EntityId);
		shovelerViewAddEntityPosition(view, op.EntityId, -op.Data.coords().x(), op.Data.coords().y(), op.Data.coords().z());
	});

	dispatcher.OnComponentUpdate<Position>([&](const worker::ComponentUpdateOp<Position>& op) {
		shovelerLogTrace("Updating position for entity %lld.", op.EntityId);
		if(op.Update.coords()) {
			const Coordinates& coordinates = *op.Update.coords();
			shovelerViewUpdateEntityPosition(view, op.EntityId, -coordinates.x(), coordinates.y(), coordinates.z());
		}
	});

	PositionUpdateRequestContext positionUpdateRequestContext{&connection};
	dispatcher.OnAuthorityChange<Position>([&](const worker::AuthorityChangeOp& op) {
		shovelerLogInfo("Changing position authority for entity %lld to %d.", op.EntityId, op.Authority);
		if (op.Authority == worker::Authority::kAuthoritative) {
			shovelerViewDelegatePosition(view, op.EntityId, requestPositionUpdate, &positionUpdateRequestContext);
		} else if (op.Authority == worker::Authority::kNotAuthoritative) {
			shovelerViewUndelegatePosition(view, op.EntityId);
		}
	});

	dispatcher.OnRemoveComponent<Position>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing position from entity %lld.", op.EntityId);
		shovelerViewRemoveEntityPosition(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Model>([&](const worker::AddComponentOp<Model>& op) {
		shovelerLogInfo("Adding model to entity %lld.", op.EntityId);
		ShovelerViewModelConfiguration modelConfiguration;
		modelConfiguration.drawable = createDrawableConfiguration(op.Data.drawable());
		modelConfiguration.material = createMaterialConfiguration(op.Data.material());
		modelConfiguration.rotation = ShovelerVector3{op.Data.rotation().x(), op.Data.rotation().y(), op.Data.rotation().z()};
		modelConfiguration.scale = ShovelerVector3{op.Data.scale().x(), op.Data.scale().y(), op.Data.scale().z()};
		modelConfiguration.visible = op.Data.visible();
		modelConfiguration.emitter = op.Data.emitter();
		modelConfiguration.screenspace = op.Data.screenspace();
		modelConfiguration.castsShadow = op.Data.casts_shadow();
		modelConfiguration.polygonMode = getPolygonMode(op.Data.polygon_mode());
		shovelerViewAddEntityModel(view, op.EntityId, modelConfiguration);
	});

	dispatcher.OnComponentUpdate<Model>([&](const worker::ComponentUpdateOp<Model>& op) {
		shovelerLogInfo("Updating model for entity %lld.", op.EntityId);

		if(op.Update.drawable()) {
			ShovelerViewDrawableConfiguration drawableConfiguration = createDrawableConfiguration(*op.Update.drawable());
			shovelerViewUpdateEntityModelDrawable(view, op.EntityId, drawableConfiguration);
		}

		if(op.Update.material()) {
			ShovelerViewMaterialConfiguration materialConfiguration = createMaterialConfiguration(*op.Update.material());
			shovelerViewUpdateEntityModelMaterial(view, op.EntityId, materialConfiguration);
		}

		if(op.Update.rotation()) {
			ShovelerVector3 rotation{op.Update.rotation()->x(), op.Update.rotation()->y(), op.Update.rotation()->z()};
			shovelerViewUpdateEntityModelRotation(view, op.EntityId, rotation);
		}

		if(op.Update.scale()) {
			ShovelerVector3 scale{-op.Update.scale()->x(), op.Update.scale()->y(), op.Update.scale()->z()};
			shovelerViewUpdateEntityModelScale(view, op.EntityId, scale);
		}

		if(op.Update.visible()) {
			shovelerViewUpdateEntityModelVisible(view, op.EntityId, *op.Update.visible());
		}

		if(op.Update.emitter()) {
			shovelerViewUpdateEntityModelEmitter(view, op.EntityId, *op.Update.emitter());
		}

		if(op.Update.screenspace()) {
			shovelerViewUpdateEntityModelScreenspace(view, op.EntityId, *op.Update.screenspace());
		}

		if(op.Update.polygon_mode()) {
			GLuint polygonMode = getPolygonMode(*op.Update.polygon_mode());
			shovelerViewUpdateEntityModelPolygonMode(view, op.EntityId, polygonMode);
		}
	});

	dispatcher.OnRemoveComponent<Model>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing model from entity %lld.", op.EntityId);
		shovelerViewRemoveEntityModel(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Light>([&](const worker::AddComponentOp<Light>& op) {
		shovelerLogInfo("Adding light to entity %lld.", op.EntityId);

		ShovelerViewLightConfiguration lightConfiguration;
		switch(op.Data.type()) {
			case LightType::SPOT:
				lightConfiguration.type = SHOVELER_VIEW_LIGHT_TYPE_SPOT;
				break;
			case LightType::POINT:
				lightConfiguration.type = SHOVELER_VIEW_LIGHT_TYPE_POINT;
				break;
			default:
				shovelerLogWarning("Tried to create light configuration with invalid light type %d, defaulting to point.", op.Data.type());
				lightConfiguration.type = SHOVELER_VIEW_LIGHT_TYPE_POINT;
				break;
		}

		lightConfiguration.width = (int) op.Data.width();
		lightConfiguration.height = (int) op.Data.height();
		lightConfiguration.samples = (GLsizei) op.Data.samples();
		lightConfiguration.ambientFactor = op.Data.ambient_factor();
		lightConfiguration.exponentialFactor = op.Data.exponential_factor();
		lightConfiguration.color = ShovelerVector3{op.Data.color().r(), op.Data.color().g(), op.Data.color().b()};
		shovelerViewAddEntityLight(view, op.EntityId, lightConfiguration);
	});

	dispatcher.OnRemoveComponent<Light>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing light from entity %lld.", op.EntityId);
		shovelerViewRemoveEntityLight(view, op.EntityId);
	});

	worker::query::EntityQuery bootstrapEntityQuery{worker::query::ComponentConstraint{Bootstrap::ComponentId}, worker::query::SnapshotResultType{{{Bootstrap::ComponentId}}}};
	worker::RequestId<worker::EntityQueryRequest> bootstrapQueryRequestId = connection.SendEntityQueryRequest(bootstrapEntityQuery, {});

	dispatcher.OnEntityQueryResponse([&](const worker::EntityQueryResponseOp& op) {
		shovelerLogInfo("Received entity query response for request %lld.", op.RequestId.Id);
		if(op.RequestId == bootstrapQueryRequestId && op.Result.size() > 0) {
			bootstrapEntityId = op.Result.begin()->first;
			shovelerLogInfo("Received bootstrap query response containing entity %lld.", bootstrapEntityId);
			worker::RequestId<worker::OutgoingCommandRequest<CreateClientEntity>> createClientEntityRequestId = connection.SendCommandRequest<CreateClientEntity>(bootstrapEntityId, {}, {});
			shovelerLogInfo("Sent create client entity request with id %d.", createClientEntityRequestId.Id);
		}
	});

	dispatcher.OnCommandResponse<CreateClientEntity>([&](const worker::CommandResponseOp<CreateClientEntity>& op) {
		shovelerLogInfo("Received create client entity command response %d with status code %d.", op.RequestId.Id, op.StatusCode);
	});

	while(shovelerGameIsRunning(game) && !disconnected) {
		dispatcher.Process(connection.GetOpList(0));
		shovelerGameRenderFrame(game);
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	shovelerInputRemoveMouseButtonCallback(game->input, mouseButtonCallback);

	shovelerViewFree(view);

	shovelerViewDrawablesFree(drawables);

	shovelerCameraPerspectiveDetachController(game->camera);
	shovelerControllerFree(controller);

	shovelerCameraFree(game->camera);
	shovelerSceneFree(game->scene);

	shovelerGameFree(game);

	shovelerGlobalUninit();
	shovelerLogTerminate();
}

static worker::Option<worker::Connection> connect(int argc, char **argv, const worker::ComponentRegistry& components)
{
	worker::ConnectionParameters connectionParameters;
	connectionParameters.WorkerType = "ShovelerClient";
	connectionParameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;

	std::string launcherPrefix = "spatialos.launch:";
	if(g_str_has_prefix(argv[1], launcherPrefix.c_str())) {
		const char *launcherString = argv[1] + launcherPrefix.size();
		gchar **projectNameSplit = g_strsplit(launcherString, "-", 2);
		if(projectNameSplit[0] == NULL || projectNameSplit[1] == NULL) {
			shovelerLogError("Failed to extract project name from launcher string: %s", launcherString);
			return {};
		}
		std::string projectName{projectNameSplit[0]};
		std::string afterProjectName{projectNameSplit[1]};
		g_strfreev(projectNameSplit);

		gchar **deploymentNameSplit = g_strsplit(afterProjectName.c_str(), "?", 2);
		if(deploymentNameSplit[0] == NULL || deploymentNameSplit[1] == NULL) {
			shovelerLogError("Failed to extract deployment name from launcher string: %s", afterProjectName.c_str());
			return {};
		}
		std::string deploymentName{deploymentNameSplit[0]};
		std::string afterDeploymentName{deploymentNameSplit[1]};
		g_strfreev(deploymentNameSplit);

		std::string tokenPrefix = "token=";
		if(!g_str_has_prefix(afterDeploymentName.c_str(), tokenPrefix.c_str())) {
			shovelerLogError("Failed to extract auth token from launcher string: %s", afterDeploymentName.c_str());
			return {};
		}
		std::string authToken{afterDeploymentName.substr(tokenPrefix.size())};

		shovelerLogInfo("Connecting to cloud deployment...\n\tProject name: %s\n\tDeployment name: %s\n\tAuth token: %s", projectName.c_str(), deploymentName.c_str(), authToken.c_str());

		worker::LocatorParameters locatorParameters;
		locatorParameters.ProjectName = projectName;
		locatorParameters.CredentialsType = worker::LocatorCredentialsType::kLoginToken;
		locatorParameters.LoginToken = worker::LoginTokenCredentials{authToken};
		worker::Locator locator{"locator.improbable.io", locatorParameters};

		auto queueStatusCallback = [&](const worker::QueueStatus& queueStatus) {
			if (!queueStatus.Error.empty()) {
				shovelerLogError("Error while queueing: %s", queueStatus.Error->c_str());
				return false;
			}
			shovelerLogInfo("Current position in login queue: %d", queueStatus.PositionInQueue);
			return true;
		};

		connectionParameters.Network.UseExternalIp = true;
		return locator.ConnectAsync(components, deploymentName, connectionParameters, queueStatusCallback).Get();
	} else {
		if (argc != 4) {
			shovelerLogError("Usage:\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>", argv[0], argv[0]);
			return {};
		}

		const std::string workerId = argv[1];
		const std::string hostname = argv[2];
		const std::uint16_t port = static_cast<std::uint16_t>(std::stoi(argv[3]));

		shovelerLogInfo("Connecting to local deployment...\n\tWorker ID: %s\n\tAddress: %s:%d", workerId.c_str(), hostname.c_str(), port);
		return worker::Connection::ConnectAsync(components, hostname, port, workerId, connectionParameters).Get();
	}
}

static void updateGame(ShovelerGame *game, double dt)
{
	shovelerControllerUpdate(controller, dt);
	shovelerCameraUpdateView(game->camera);
}

static void requestPositionUpdate(ShovelerViewComponent *component, double x, double y, double z, void *positionUpdateRequestContextPointer)
{
	PositionUpdateRequestContext *context = (PositionUpdateRequestContext *) positionUpdateRequestContextPointer;

	Position::Update positionUpdate;
	positionUpdate.set_coords({-x, y, z});
	context->connection->SendComponentUpdate<Position>(component->entity->entityId, positionUpdate);
}

static void clientPingTick(void *clientPingTickContextPointer)
{
	ClientPingTickContext *context = (ClientPingTickContext *) clientPingTickContextPointer;

	int64_t now = g_get_monotonic_time();
	context->connection->SendCommandRequest<ClientPing>(context->bootstrapEntityId, {context->clientEntityId, now}, {});
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

static ShovelerViewDrawableConfiguration createDrawableConfiguration(const Drawable& drawable)
{
	ShovelerViewDrawableConfiguration drawableConfiguration;
	switch(drawable.type()) {
		case DrawableType::CUBE:
			drawableConfiguration.type = SHOVELER_VIEW_DRAWABLE_TYPE_CUBE;
			break;
		case DrawableType::QUAD:
			drawableConfiguration.type = SHOVELER_VIEW_DRAWABLE_TYPE_QUAD;
			break;
		case DrawableType::POINT:
			drawableConfiguration.type = SHOVELER_VIEW_DRAWABLE_TYPE_POINT;
			break;
		default:
			shovelerLogWarning("Tried to create drawable configuration with invalid drawable type %d, defaulting to cube.", drawable.type());
			drawableConfiguration.type = SHOVELER_VIEW_DRAWABLE_TYPE_CUBE;
			break;
	}
	return drawableConfiguration;
}

static ShovelerViewMaterialConfiguration createMaterialConfiguration(const Material& material)
{
	ShovelerViewMaterialConfiguration materialConfiguration;
	switch(material.type()) {
		case MaterialType::COLOR:
			materialConfiguration.type = SHOVELER_VIEW_MATERIAL_TYPE_COLOR;
			if(material.color()) {
				materialConfiguration.color = ShovelerVector3{material.color()->r(), material.color()->g(), material.color()->b()};
			} else {
				shovelerLogWarning("Tried to create color material configuration without color, defaulting to white.");
				materialConfiguration.color = ShovelerVector3{1.0f, 1.0f, 1.0f};
			}
			break;
		case MaterialType::TEXTURE:
			materialConfiguration.type = SHOVELER_VIEW_MATERIAL_TYPE_TEXTURE;
			if(material.texture()) {
				materialConfiguration.texture = material.texture()->c_str();
			} else {
				shovelerLogWarning("Tried to create texture material configuration without texture, defaulting to null.");
				materialConfiguration.texture = NULL;
			}
			break;
		case MaterialType::PARTICLE:
			materialConfiguration.type = SHOVELER_VIEW_MATERIAL_TYPE_PARTICLE;
			if(material.color()) {
				materialConfiguration.color = ShovelerVector3{material.color()->r(), material.color()->g(), material.color()->b()};
			} else {
				shovelerLogWarning("Tried to create particle material configuration without color, defaulting to white.");
				materialConfiguration.color = ShovelerVector3{1.0f, 1.0f, 1.0f};
			}
			break;
		default:
			shovelerLogWarning("Tried to create material configuration with invalid material type %d, defaulting to pink color.", material.type());
			materialConfiguration.type = SHOVELER_VIEW_MATERIAL_TYPE_COLOR;
			materialConfiguration.color = ShovelerVector3{1.0f, 0.41f, 0.71f};
			break;
	}
	return materialConfiguration;
}

static GLuint getPolygonMode(PolygonMode polygonMode)
{
	switch(polygonMode) {
		case PolygonMode::FILL:
			return GL_FILL;
		case PolygonMode::LINE:
			return GL_LINE;
		case PolygonMode::POINT:
			return GL_POINT;
		default:
			shovelerLogWarning("Tried to retrieve invalid polygon mode %d, defaulting to FILL.", polygonMode);
			return GL_FILL;
	}
}
