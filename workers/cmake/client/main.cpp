#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <camera/identity.h>
#include <game.h>
#include <log.h>
#include <types.h>

#include "worker_view.h"
}

using improbable::Coordinates;
using improbable::Position;

static void updateGame(ShovelerGame *game, double dt);

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

	shovelerLogInfo("Connecting as worker %s to %s:%d.", workerId.c_str(), hostname.c_str(), port);
	worker::Connection connection = worker::Connection::ConnectAsync(hostname, port, workerId, parameters).Get();
	bool disconnected = false;
	worker::Dispatcher dispatcher;

	ShovelerSpatialOsWorkerView *view = shovelerSpatialOsWorkerViewCreate();

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
		ShovelerVector3 position{(float) op.Data.coords().x(), (float) op.Data.coords().y(), (float) op.Data.coords().z()};
		shovelerSpatialOsWorkerViewAddEntityPosition(view, op.EntityId, position);
	});

	dispatcher.OnComponentUpdate<Position>([&](const worker::ComponentUpdateOp<Position>& op) {
		shovelerLogInfo("Updating position for entity %lld.", op.EntityId);
		if(op.Update.coords()) {
			const Coordinates& coordinates = *op.Update.coords();
			ShovelerVector3 position{(float) coordinates.x(), (float) coordinates.y(), (float) coordinates.z()};
			shovelerSpatialOsWorkerViewUpdateEntityPosition(view, op.EntityId, position);
		}
	});

	dispatcher.OnRemoveComponent<Position>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing position from entity %lld.", op.EntityId);
		shovelerSpatialOsWorkerViewRemoveEntityPosition(view, op.EntityId);
	});

	game->camera = shovelerCameraIdentityCreate();
	game->scene = shovelerSceneCreate();
	game->update = updateGame;

	while(shovelerGameIsRunning(game) && !disconnected) {
		dispatcher.Process(connection.GetOpList(0));
		shovelerGameRenderFrame(game);
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	shovelerCameraFree(game->camera);
	shovelerSceneFree(game->scene);

	shovelerSpatialOsWorkerViewFree(view);

	shovelerGameFree(game);
}

static void updateGame(ShovelerGame *game, double dt)
{

}
