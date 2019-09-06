#include <stdlib.h> // srand
#include <string.h> // memset
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

#include "configuration.h"
#include "connect.h"

typedef struct {
	Worker_Connection *connection;
	ShovelerGame *game;
	ShovelerClientConfiguration *clientConfiguration;
	long long int clientEntityId;
} ClientContext;

static const long long int bootstrapEntityId = 1;
static const long long int bootstrapComponentId = 1334;
static const long long int clientComponentId = 1335;
static const int64_t clientPingTimeoutMs = 1000;

static void handleLogMessageOp(const Worker_LogMessageOp *op, bool *disconnected);
static void updateGame(ShovelerGame *game, double dt);
static void clientPingTick(void *clientContextPointer);

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

	Worker_CommandRequest createClientEntityCommandRequest;
	memset(&createClientEntityCommandRequest, 0, sizeof(Worker_CommandRequest));
	createClientEntityCommandRequest.component_id = bootstrapComponentId;
	createClientEntityCommandRequest.command_index = 1;
	createClientEntityCommandRequest.schema_type = Schema_CreateCommandRequest();

	Worker_RequestId createClientEntityCommandRequestId = Worker_Connection_SendCommandRequest(
		connection,
		bootstrapEntityId,
		&createClientEntityCommandRequest,
		/* timeout_millis */ NULL,
		/* command_parameters */ NULL);
	if(createClientEntityCommandRequestId < 0) {
		shovelerLogError("Failed to send create entity command.");
		Worker_Connection_Destroy(connection);
		return EXIT_FAILURE;
	}
	shovelerLogInfo("Sent create entity command request %lld.", createClientEntityCommandRequestId);

	ShovelerClientConfiguration clientConfiguration;
	if(!shovelerClientGetWorkerConfiguration(connection, &clientConfiguration)) {
		shovelerLogError("Failed to retreive client configuration.");
		Worker_Connection_Destroy(connection);
		return EXIT_FAILURE;
	}

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

	ClientContext context;
	context.connection = connection;
	context.game = game;
	context.clientConfiguration = &clientConfiguration;

	game->controller->lockMoveX = clientConfiguration.controllerLockMoveX;
	game->controller->lockMoveY = clientConfiguration.controllerLockMoveY;
	game->controller->lockMoveZ = clientConfiguration.controllerLockMoveZ;
	game->controller->lockTiltX = clientConfiguration.controllerLockTiltX;
	game->controller->lockTiltY = clientConfiguration.controllerLockTiltY;

	bool disconnected = false;
	ShovelerExecutorCallback *clientPingTickCallback = NULL;

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
				case WORKER_OP_TYPE_AUTHORITY_CHANGE: {
					Worker_AuthorityChangeOp *authorityChangeOp = &op->op.authority_change;
					if(authorityChangeOp->component_id == clientComponentId) {
						if(authorityChangeOp->authority == WORKER_AUTHORITY_AUTHORITATIVE) {
							shovelerLogInfo("Gained client authority over entity %lld.", authorityChangeOp->entity_id);
							context.clientEntityId = authorityChangeOp->entity_id;
							clientPingTickCallback = shovelerExecutorSchedulePeriodic(game->updateExecutor, 0, clientPingTimeoutMs, clientPingTick, &context);
						} else if(authorityChangeOp->authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE) {
							shovelerLogWarning("Lost client authority over entity %lld.", authorityChangeOp->entity_id);
							context.clientEntityId = 0;
							shovelerExecutorRemoveCallback(game->updateExecutor, clientPingTickCallback);
						}
					} else {
						shovelerLogInfo("Authority over component %u of entity %lld changed to %u.", authorityChangeOp->component_id, authorityChangeOp->entity_id, authorityChangeOp->authority);
					}
				} break;
				case WORKER_OP_TYPE_COMPONENT_UPDATE:
					shovelerLogInfo("WORKER_OP_TYPE_COMPONENT_UPDATE");
					break;
				case WORKER_OP_TYPE_COMMAND_REQUEST:
					shovelerLogInfo("WORKER_OP_TYPE_COMMAND_REQUEST");
					break;
				case WORKER_OP_TYPE_COMMAND_RESPONSE:
					if (op->op.command_response.request_id == createClientEntityCommandRequestId) {
						shovelerLogInfo(
							"Create client entity command request %lld completed with code %u: %s",
							op->op.command_response.request_id,
							op->op.command_response.status_code,
							op->op.command_response.message);
					} else {
						shovelerLogTrace(
							"Entity command %lld to %lld completed with code %u: %s",
							op->op.command_response.request_id,
							op->op.command_response.entity_id,
							op->op.command_response.status_code,
							op->op.command_response.message);
					}
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

static void clientPingTick(void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;

	Worker_CommandRequest clientPingCommandRequest;
	memset(&clientPingCommandRequest, 0, sizeof(Worker_CommandRequest));
	clientPingCommandRequest.component_id = bootstrapComponentId;
	clientPingCommandRequest.command_index = 2;
	clientPingCommandRequest.schema_type = Schema_CreateCommandRequest();
	Schema_Object *clientPingCommandRequestObject = Schema_GetCommandRequestObject(clientPingCommandRequest.schema_type);
	Schema_AddEntityId(clientPingCommandRequestObject, 1, context->clientEntityId);

	Worker_RequestId clientPingCommandRequestId = Worker_Connection_SendCommandRequest(
		context->connection,
		bootstrapEntityId,
		&clientPingCommandRequest,
		/* timeout_millis */ NULL,
		/* command_parameters */ NULL);
	if(clientPingCommandRequestId < 0) {
		shovelerLogWarning("Failed to send client ping command.");
	}
	shovelerLogTrace("Sent client ping command request %lld.", clientPingCommandRequestId);
}
