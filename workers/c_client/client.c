#include <stdlib.h> // srand
#include <time.h> // time

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <shoveler/constants.h>
#include <shoveler/game.h>
#include <shoveler/global.h>
#include <shoveler/log.h>
#include <shoveler/resources/image_png.h>
#include <shoveler/resources.h>
#include <shoveler/types.h>

#include "connect.h"

typedef struct {
	ShovelerGameControllerSettings controllerSettings;
	bool controllerLockMoveX;
	bool controllerLockMoveY;
	bool controllerLockMoveZ;
	bool controllerLockTiltX;
	bool controllerLockTiltY;
	ShovelerCoordinateMapping positionMappingX;
	ShovelerCoordinateMapping positionMappingY;
	ShovelerCoordinateMapping positionMappingZ;
	bool hidePlayerClientEntityModel;
} ClientConfiguration;

static void handleLogMessageOp(const Worker_LogMessageOp *op, bool *disconnected);
static void updateGame(ShovelerGame *game, double dt);

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

	if (argc != 1 && argc != 2 && argc != 4 && argc != 5) {
		shovelerLogError("Usage:\n\t%s\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>\n\t%s <locator hostname> <project name> <deployment name> <login token>", argv[0], argv[0], argv[0], argv[0]);
		return EXIT_FAILURE;
	}

	Worker_ComponentVtable componentVtable = {0};

	Worker_ConnectionParameters connectionParameters = Worker_DefaultConnectionParameters();
	connectionParameters.worker_type = "ShovelerCClient";
	connectionParameters.network.connection_type = WORKER_NETWORK_CONNECTION_TYPE_TCP;
	connectionParameters.default_component_vtable = &componentVtable;

	shovelerLogInfo("Using SpatialOS C Worker SDK '%s'.", Worker_ApiVersionStr());
	Worker_Connection *connection = shovelerClientConnect(argc, argv, &connectionParameters);
	if(connection == NULL) {
		shovelerLogError("Failed to connect to SpatialOS deployment.");
		return EXIT_FAILURE;
	}
	shovelerLogInfo("Connected to SpatialOS deployment!");

	ClientConfiguration clientConfiguration;
	clientConfiguration.controllerSettings.frame.position = shovelerVector3(0, 0, 0);
	clientConfiguration.controllerSettings.frame.direction = shovelerVector3(0, 0, 1);
	clientConfiguration.controllerSettings.frame.up = shovelerVector3(0, 1, 0);
	clientConfiguration.controllerLockMoveX = false;
	clientConfiguration.controllerLockMoveY = false;
	clientConfiguration.controllerLockMoveZ = false;
	clientConfiguration.controllerLockTiltX = false;
	clientConfiguration.controllerLockTiltY = false;
	clientConfiguration.controllerSettings.moveFactor = 2.0f;
	clientConfiguration.controllerSettings.tiltFactor = 0.0005f;
	clientConfiguration.controllerSettings.boundingBoxSize2 = 0.0f;
	clientConfiguration.controllerSettings.boundingBoxSize3 = 0.0f;
	clientConfiguration.positionMappingX = SHOVELER_COORDINATE_MAPPING_POSITIVE_X;
	clientConfiguration.positionMappingY = SHOVELER_COORDINATE_MAPPING_POSITIVE_Y;
	clientConfiguration.positionMappingZ = SHOVELER_COORDINATE_MAPPING_POSITIVE_Z;
	clientConfiguration.hidePlayerClientEntityModel = true;

	ShovelerGameCameraSettings cameraSettings;
	cameraSettings.frame = clientConfiguration.controllerSettings.frame;
	cameraSettings.projection.fieldOfViewY = 2.0f * SHOVELER_PI * 50.0f / 360.0f;
	cameraSettings.projection.aspectRatio = (float) windowSettings.windowedWidth / windowSettings.windowedHeight;
	cameraSettings.projection.nearClippingPlane = 0.01;
	cameraSettings.projection.farClippingPlane = 1000;

	ShovelerGame *game = shovelerGameCreate(updateGame, &windowSettings, &cameraSettings, &clientConfiguration.controllerSettings);
	if(game == NULL) {
		return EXIT_FAILURE;
	}
	ShovelerView *view = game->view;

	game->controller->lockMoveX = clientConfiguration.controllerLockMoveX;
	game->controller->lockMoveY = clientConfiguration.controllerLockMoveY;
	game->controller->lockMoveZ = clientConfiguration.controllerLockMoveZ;
	game->controller->lockTiltX = clientConfiguration.controllerLockTiltX;
	game->controller->lockTiltY = clientConfiguration.controllerLockTiltY;

	bool disconnected = false;

	ShovelerResources *resources = shovelerResourcesCreate(/* TODO: on demand resource loading */ NULL, NULL);
	shovelerResourcesImagePngRegister(resources);

	while(shovelerGameIsRunning(game) && !disconnected) {
		Worker_OpList *opList = Worker_Connection_GetOpList(connection, 0);
		for(size_t i = 0; i < opList->op_count; ++i) {
			Worker_Op *op = &opList->ops[i];
			switch(op->op_type) {
				case WORKER_OP_TYPE_DISCONNECT:
					shovelerLogInfo("Disconnected from SpatialOS with code %d: %s", op->op.disconnect.reason, op->op.disconnect.connection_status_code);
					disconnected = true;
					break;
				case WORKER_OP_TYPE_FLAG_UPDATE:
					shovelerLogInfo("WORKER_OP_TYPE_FLAG_UPDATE");
					break;
				case WORKER_OP_TYPE_LOG_MESSAGE:
					handleLogMessageOp(&op->op.log_message, &disconnected);
					break;
				case WORKER_OP_TYPE_METRICS:
					Worker_Connection_SendMetrics(connection, &op->op.metrics.metrics);
					break;
				case WORKER_OP_TYPE_CRITICAL_SECTION:
					shovelerLogInfo("WORKER_OP_TYPE_CRITICAL_SECTION");
					break;
				case WORKER_OP_TYPE_ADD_ENTITY:
					shovelerLogInfo("WORKER_OP_TYPE_ADD_ENTITY");
					break;
				case WORKER_OP_TYPE_REMOVE_ENTITY:
					shovelerLogInfo("WORKER_OP_TYPE_REMOVE_ENTITY");
					break;
				case WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE:
					shovelerLogInfo("WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE");
					break;
				case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
					shovelerLogInfo("WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE");
					break;
				case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
					shovelerLogInfo("WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE");
					break;
				case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
					shovelerLogInfo("WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE");
					break;
				case WORKER_OP_TYPE_ADD_COMPONENT:
					shovelerLogInfo("WORKER_OP_TYPE_ADD_COMPONENT");
					break;
				case WORKER_OP_TYPE_REMOVE_COMPONENT:
					shovelerLogInfo("WORKER_OP_TYPE_REMOVE_COMPONENT");
					break;
				case WORKER_OP_TYPE_AUTHORITY_CHANGE:
					shovelerLogInfo("WORKER_OP_TYPE_AUTHORITY_CHANGE");
					break;
				case WORKER_OP_TYPE_COMPONENT_UPDATE:
					shovelerLogInfo("WORKER_OP_TYPE_COMPONENT_UPDATE");
					break;
				case WORKER_OP_TYPE_COMMAND_REQUEST:
					shovelerLogInfo("WORKER_OP_TYPE_COMMAND_REQUEST");
					break;
				case WORKER_OP_TYPE_COMMAND_RESPONSE:
					shovelerLogInfo("WORKER_OP_TYPE_COMMAND_RESPONSE");
					break;
			}
		}
		Worker_OpList_Destroy(opList);

		shovelerGameRenderFrame(game);
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	Worker_Connection_Destroy(connection);

	shovelerGameFree(game);
	shovelerResourcesFree(resources);
	shovelerGlobalUninit();
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}

static void handleLogMessageOp(const Worker_LogMessageOp *op, bool *disconnected)
{
	switch(op->level) {
		case WORKER_LOG_LEVEL_DEBUG:
			shovelerLogTrace("[Worker SDK] %s", op->message);
			break;
		case WORKER_LOG_LEVEL_INFO:
			shovelerLogInfo("[Worker SDK] %s", op->message);
			break;
		case WORKER_LOG_LEVEL_WARN:
			shovelerLogWarning("[Worker SDK] %s", op->message);
			break;
		case WORKER_LOG_LEVEL_ERROR:
			shovelerLogError("[Worker SDK] %s", op->message);
			break;
		case WORKER_LOG_LEVEL_FATAL:
			shovelerLogError("Disconnecting due to fatal Worker SDK error: %s", op->message);
			*disconnected = true;
			break;
	}
}

static void updateGame(ShovelerGame *game, double dt)
{
	shovelerCameraUpdateView(game->camera);
}
