#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <shoveler/camera/perspective.h>
#include <shoveler/constants.h>
#include <shoveler/controller.h>
#include <shoveler/game.h>
#include <shoveler/log.h>
#include <shoveler/types.h>

#include "worker_view.h"
}

using shoveler::Drawable;
using shoveler::DrawableType;
using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Model;
using shoveler::Light;
using shoveler::LightType;
using improbable::Coordinates;
using improbable::Position;

static void updateGame(ShovelerGame *game, double dt);
static ShovelerSpatialOsWorkerViewDrawableConfiguration createDrawableConfiguration(const Drawable& drawable);
static ShovelerSpatialOsWorkerViewMaterialConfiguration createMaterialConfiguration(const Material& material);

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

	ShovelerSpatialOsWorkerView *view = shovelerSpatialOsWorkerViewCreate(game->scene);

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		shovelerLogError("Disconnected from SpatialOS: %s", op.Reason.c_str());
		disconnected = true;
	});

	dispatcher.OnAddEntity([&](const worker::AddEntityOp& op) {
		shovelerLogInfo("Adding entity %lld.", op.EntityId);
		shovelerSpatialOsWorkerViewAddEntity(view, op.EntityId);
	});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op) {
		shovelerLogInfo("Removing entity %lld.", op.EntityId);
		shovelerSpatialOsWorkerViewRemoveEntity(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Position>([&](const worker::AddComponentOp<Position>& op) {
		shovelerLogInfo("Adding position to entity %lld.", op.EntityId);
		ShovelerVector3 position{(float) -op.Data.coords().x(), (float) op.Data.coords().y(), (float) op.Data.coords().z()};
		shovelerSpatialOsWorkerViewAddEntityPosition(view, op.EntityId, position);
	});

	dispatcher.OnComponentUpdate<Position>([&](const worker::ComponentUpdateOp<Position>& op) {
		shovelerLogInfo("Updating position for entity %lld.", op.EntityId);
		if(op.Update.coords()) {
			const Coordinates& coordinates = *op.Update.coords();
			ShovelerVector3 position{(float) -coordinates.x(), (float) coordinates.y(), (float) coordinates.z()};
			shovelerSpatialOsWorkerViewUpdateEntityPosition(view, op.EntityId, position);
		}
	});

	dispatcher.OnRemoveComponent<Position>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing position from entity %lld.", op.EntityId);
		shovelerSpatialOsWorkerViewRemoveEntityPosition(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Model>([&](const worker::AddComponentOp<Model>& op) {
		shovelerLogInfo("Adding model to entity %lld.", op.EntityId);
		ShovelerSpatialOsWorkerViewDrawableConfiguration drawableConfiguration = createDrawableConfiguration(op.Data.drawable());
		ShovelerSpatialOsWorkerViewMaterialConfiguration materialConfiguration = createMaterialConfiguration(op.Data.material());
		shovelerSpatialOsWorkerViewAddEntityModel(view, op.EntityId, drawableConfiguration, materialConfiguration);
	});

	dispatcher.OnComponentUpdate<Model>([&](const worker::ComponentUpdateOp<Model>& op) {
		shovelerLogInfo("Updating model for entity %lld.", op.EntityId);

		ShovelerSpatialOsWorkerViewDrawableConfiguration drawableConfiguration;
		ShovelerSpatialOsWorkerViewDrawableConfiguration *optionalDrawableConfiguration = NULL;
		if(op.Update.drawable()) {
			drawableConfiguration = createDrawableConfiguration(*op.Update.drawable());
			optionalDrawableConfiguration = &drawableConfiguration;
		}

		ShovelerSpatialOsWorkerViewMaterialConfiguration materialConfiguration;
		ShovelerSpatialOsWorkerViewMaterialConfiguration *optionalMaterialConfiguration = NULL;
		if(op.Update.material()) {
			materialConfiguration = createMaterialConfiguration(*op.Update.material());
			optionalMaterialConfiguration = &materialConfiguration;
		}

		shovelerSpatialOsWorkerViewUpdateEntityModel(view, op.EntityId, optionalDrawableConfiguration, optionalMaterialConfiguration);
	});

	dispatcher.OnRemoveComponent<Model>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing model from entity %lld.", op.EntityId);
		shovelerSpatialOsWorkerViewRemoveEntityModel(view, op.EntityId);
	});

	dispatcher.OnAddComponent<Light>([&](const worker::AddComponentOp<Light>& op) {
		shovelerLogInfo("Adding light to entity %lld.", op.EntityId);

		ShovelerSpatialOsWorkerViewLightConfiguration lightConfiguration;
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
		shovelerSpatialOsWorkerViewAddEntityLight(view, op.EntityId, lightConfiguration);
	});

	dispatcher.OnRemoveComponent<Light>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing light from entity %lld.", op.EntityId);
		shovelerSpatialOsWorkerViewRemoveEntityLight(view, op.EntityId);
	});

	while(shovelerGameIsRunning(game) && !disconnected) {
		dispatcher.Process(connection.GetOpList(0));
		shovelerGameRenderFrame(game);
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	shovelerSpatialOsWorkerViewFree(view);

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

static ShovelerSpatialOsWorkerViewDrawableConfiguration createDrawableConfiguration(const Drawable& drawable)
{
	ShovelerSpatialOsWorkerViewDrawableConfiguration drawableConfiguration;
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

static ShovelerSpatialOsWorkerViewMaterialConfiguration createMaterialConfiguration(const Material& material)
{
	ShovelerSpatialOsWorkerViewMaterialConfiguration materialConfiguration;
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
