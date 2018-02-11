#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <shoveler/spatialos/worker/view/base/view.h>
#include <shoveler/spatialos/worker/view/base/position.h>
#include <shoveler/spatialos/worker/view/visual/drawables.h>
#include <shoveler/spatialos/worker/view/visual/light.h>
#include <shoveler/spatialos/worker/view/visual/model.h>
#include <shoveler/spatialos/worker/view/visual/scene.h>
#include <shoveler/camera/perspective.h>
#include <shoveler/constants.h>
#include <shoveler/controller.h>
#include <shoveler/game.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
}

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

static void updateGame(ShovelerGame *game, double dt);
static ShovelerSpatialosWorkerViewDrawableConfiguration createDrawableConfiguration(const Drawable& drawable);
static ShovelerSpatialosWorkerViewMaterialConfiguration createMaterialConfiguration(const Material& material);
static GLuint getPolygonMode(PolygonMode polygonMode);

static ShovelerController *controller;

int main(int argc, char **argv) {
	if (argc != 4) {
		return 1;
	}

	const char *windowTitle = "ShovelerClient";
	bool fullscreen = false;
	bool vsync = true;
	int samples = 4;
	int width = 640;
	int height = 480;

	ShovelerGame *game = shovelerGameCreate(windowTitle, width, height, samples, fullscreen, vsync);
	if(game == NULL) {
		return EXIT_FAILURE;
	}

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "ShovelerClient";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
	parameters.Network.UseExternalIp = false;

	const std::string workerId = argv[1];
	const std::string hostname = argv[2];
	const std::uint16_t port = static_cast<std::uint16_t>(std::stoi(argv[3]));

	const worker::ComponentRegistry& components = worker::Components<
		shoveler::Light,
		shoveler::Model,
		improbable::Position>{};

	shovelerLogInfo("Connecting as worker %s to %s:%d.", workerId.c_str(), hostname.c_str(), port);
	worker::Connection connection = worker::Connection::ConnectAsync(components, hostname, port, workerId, parameters).Get();
	bool disconnected = false;
	worker::Dispatcher dispatcher{components};

	game->camera = shovelerCameraPerspectiveCreate(ShovelerVector3{0.0, 0.0, -1.0}, ShovelerVector3{0.0, 0.0, 1.0}, ShovelerVector3{0.0, 1.0, 0.0}, 2.0f * SHOVELER_PI * 50.0f / 360.0f, (float) width / height, 0.01, 1000);
	game->scene = shovelerSceneCreate();
	game->update = updateGame;

	controller = shovelerControllerCreate(game, 2.0f, 0.0005f);
	shovelerCameraPerspectiveAttachController(game->camera, controller);

	ShovelerSpatialosWorkerView *view = shovelerSpatialosWorkerViewCreate();
	shovelerSpatialosWorkerViewSetTarget(view, "scene", game->scene);

	ShovelerSpatialosWorkerViewDrawables *drawables = shovelerSpatialosWorkerViewDrawablesCreate(view);

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		shovelerLogError("Disconnected from SpatialOS: %s", op.Reason.c_str());
		disconnected = true;
	});

	dispatcher.OnAddEntity([&](const worker::AddEntityOp& op) {
		shovelerLogInfo("Adding entity %lld.", op.EntityId);
		shovelerSpatialosWorkerViewAddEntity(view, op.EntityId);
	});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op) {
		shovelerLogInfo("Removing entity %lld.", op.EntityId);
		shovelerSpatialosWorkerViewRemoveEntity(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Position>([&](const worker::AddComponentOp<Position>& op) {
		shovelerLogInfo("Adding position to entity %lld.", op.EntityId);
		shovelerSpatialosWorkerViewAddEntityPosition(view, op.EntityId, -op.Data.coords().x(), op.Data.coords().y(), op.Data.coords().z());
	});

	dispatcher.OnComponentUpdate<Position>([&](const worker::ComponentUpdateOp<Position>& op) {
		shovelerLogInfo("Updating position for entity %lld.", op.EntityId);
		if(op.Update.coords()) {
			const Coordinates& coordinates = *op.Update.coords();
			shovelerSpatialosWorkerViewUpdateEntityPosition(view, op.EntityId, -coordinates.x(), coordinates.y(), coordinates.z());
		}
	});

	dispatcher.OnRemoveComponent<Position>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing position from entity %lld.", op.EntityId);
		shovelerSpatialosWorkerViewRemoveEntityPosition(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Model>([&](const worker::AddComponentOp<Model>& op) {
		shovelerLogInfo("Adding model to entity %lld.", op.EntityId);
		ShovelerSpatialosWorkerViewModelConfiguration modelConfiguration;
		modelConfiguration.drawable = createDrawableConfiguration(op.Data.drawable());
		modelConfiguration.material = createMaterialConfiguration(op.Data.material());
		modelConfiguration.rotation = ShovelerVector3{op.Data.rotation().x(), op.Data.rotation().y(), op.Data.rotation().z()};
		modelConfiguration.scale = ShovelerVector3{op.Data.scale().x(), op.Data.scale().y(), op.Data.scale().z()};
		modelConfiguration.visible = op.Data.visible();
		modelConfiguration.emitter = op.Data.emitter();
		modelConfiguration.screenspace = op.Data.screenspace();
		modelConfiguration.castsShadow = op.Data.casts_shadow();
		modelConfiguration.polygonMode = getPolygonMode(op.Data.polygon_mode());
		shovelerSpatialosWorkerViewAddEntityModel(view, op.EntityId, modelConfiguration);
	});

	dispatcher.OnComponentUpdate<Model>([&](const worker::ComponentUpdateOp<Model>& op) {
		shovelerLogInfo("Updating model for entity %lld.", op.EntityId);

		if(op.Update.drawable()) {
			ShovelerSpatialosWorkerViewDrawableConfiguration drawableConfiguration = createDrawableConfiguration(*op.Update.drawable());
			shovelerSpatialosWorkerViewUpdateEntityModelDrawable(view, op.EntityId, drawableConfiguration);
		}

		if(op.Update.material()) {
			ShovelerSpatialosWorkerViewMaterialConfiguration materialConfiguration = createMaterialConfiguration(*op.Update.material());
			shovelerSpatialosWorkerViewUpdateEntityModelMaterial(view, op.EntityId, materialConfiguration);
		}

		if(op.Update.rotation()) {
			ShovelerVector3 rotation{op.Update.rotation()->x(), op.Update.rotation()->y(), op.Update.rotation()->z()};
			shovelerSpatialosWorkerViewUpdateEntityModelRotation(view, op.EntityId, rotation);
		}

		if(op.Update.scale()) {
			ShovelerVector3 scale{-op.Update.scale()->x(), op.Update.scale()->y(), op.Update.scale()->z()};
			shovelerSpatialosWorkerViewUpdateEntityModelScale(view, op.EntityId, scale);
		}

		if(op.Update.visible()) {
			shovelerSpatialosWorkerViewUpdateEntityModelVisible(view, op.EntityId, *op.Update.visible());
		}

		if(op.Update.emitter()) {
			shovelerSpatialosWorkerViewUpdateEntityModelEmitter(view, op.EntityId, *op.Update.emitter());
		}

		if(op.Update.screenspace()) {
			shovelerSpatialosWorkerViewUpdateEntityModelScreenspace(view, op.EntityId, *op.Update.screenspace());
		}

		if(op.Update.polygon_mode()) {
			GLuint polygonMode = getPolygonMode(*op.Update.polygon_mode());
			shovelerSpatialosWorkerViewUpdateEntityModelPolygonMode(view, op.EntityId, polygonMode);
		}
	});

	dispatcher.OnRemoveComponent<Model>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing model from entity %lld.", op.EntityId);
		shovelerSpatialosWorkerViewRemoveEntityModel(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Light>([&](const worker::AddComponentOp<Light>& op) {
		shovelerLogInfo("Adding light to entity %lld.", op.EntityId);

		ShovelerSpatialosWorkerViewLightConfiguration lightConfiguration;
		switch(op.Data.type()) {
			case LightType::SPOT:
				lightConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_SPOT;
				break;
			case LightType::POINT:
				lightConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_POINT;
				break;
			default:
				shovelerLogWarning("Tried to create light configuration with invalid light type %d, defaulting to point.", op.Data.type());
				lightConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_POINT;
				break;
		}

		lightConfiguration.width = (int) op.Data.width();
		lightConfiguration.height = (int) op.Data.height();
		lightConfiguration.samples = (GLsizei) op.Data.samples();
		lightConfiguration.ambientFactor = op.Data.ambient_factor();
		lightConfiguration.exponentialFactor = op.Data.exponential_factor();
		lightConfiguration.color = ShovelerVector3{op.Data.color().r(), op.Data.color().g(), op.Data.color().b()};
		shovelerSpatialosWorkerViewAddEntityLight(view, op.EntityId, lightConfiguration);
	});

	dispatcher.OnRemoveComponent<Light>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing light from entity %lld.", op.EntityId);
		shovelerSpatialosWorkerViewRemoveEntityLight(view, op.EntityId);
	});

	while(shovelerGameIsRunning(game) && !disconnected) {
		dispatcher.Process(connection.GetOpList(0));
		shovelerGameRenderFrame(game);
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	shovelerSpatialosWorkerViewFree(view);

	shovelerSpatialosWorkerViewDrawablesFree(drawables);

	shovelerCameraPerspectiveDetachController(game->camera, controller);
	shovelerControllerFree(controller);

	shovelerCameraFree(game->camera);
	shovelerSceneFree(game->scene);

	shovelerGameFree(game);
}

static void updateGame(ShovelerGame *game, double dt)
{
	shovelerControllerUpdate(controller, dt);
	shovelerCameraUpdateView(game->camera);
}

static ShovelerSpatialosWorkerViewDrawableConfiguration createDrawableConfiguration(const Drawable& drawable)
{
	ShovelerSpatialosWorkerViewDrawableConfiguration drawableConfiguration;
	switch(drawable.type()) {
		case DrawableType::CUBE:
			drawableConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_CUBE;
			break;
		case DrawableType::QUAD:
			drawableConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_QUAD;
			break;
		case DrawableType::POINT:
			drawableConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_POINT;
			break;
		default:
			shovelerLogWarning("Tried to create drawable configuration with invalid drawable type %d, defaulting to cube.", drawable.type());
			drawableConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_CUBE;
			break;
	}
	return drawableConfiguration;
}

static ShovelerSpatialosWorkerViewMaterialConfiguration createMaterialConfiguration(const Material& material)
{
	ShovelerSpatialosWorkerViewMaterialConfiguration materialConfiguration;
	switch(material.type()) {
		case MaterialType::COLOR:
			materialConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_COLOR;
			if(material.color()) {
				materialConfiguration.color = ShovelerVector3{material.color()->r(), material.color()->g(), material.color()->b()};
			} else {
				shovelerLogWarning("Tried to create color material configuration without color, defaulting to white.");
				materialConfiguration.color = ShovelerVector3{1.0f, 1.0f, 1.0f};
			}
			break;
		case MaterialType::TEXTURE:
			materialConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_TEXTURE;
			if(material.texture()) {
				materialConfiguration.texture = material.texture()->c_str();
			} else {
				shovelerLogWarning("Tried to create texture material configuration without texture, defaulting to null.");
				materialConfiguration.texture = NULL;
			}
			break;
		case MaterialType::PARTICLE:
			materialConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_PARTICLE;
			if(material.color()) {
				materialConfiguration.color = ShovelerVector3{material.color()->r(), material.color()->g(), material.color()->b()};
			} else {
				shovelerLogWarning("Tried to create particle material configuration without color, defaulting to white.");
				materialConfiguration.color = ShovelerVector3{1.0f, 1.0f, 1.0f};
			}
			break;
		default:
			shovelerLogWarning("Tried to create material configuration with invalid material type %d, defaulting to pink color.", material.type());
			materialConfiguration.type = SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_COLOR;
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
