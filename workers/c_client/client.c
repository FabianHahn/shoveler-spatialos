#include <stdlib.h> // srand
#include <string.h> // memset
#include <time.h> // time

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <shoveler/constants.h>
#include <shoveler/component.h>
#include <shoveler/game.h>
#include <shoveler/global.h>
#include <shoveler/log.h>
#include <shoveler/resources/image_png.h>
#include <shoveler/resources.h>
#include <shoveler/types.h>
#include <shoveler/view.h>

#include "configuration.h"
#include "connect.h"
#include "schema.h"

typedef struct {
	Worker_Connection *connection;
	ShovelerGame *game;
	ShovelerClientConfiguration *clientConfiguration;
	long long int clientEntityId;
	bool disconnected;
	ShovelerExecutorCallback *clientPingTickCallback;
} ClientContext;

static const long long int bootstrapEntityId = 1;
static const long long int bootstrapComponentId = 1334;
static const long long int clientComponentId = 1335;
static const int64_t clientPingTimeoutMs = 1000;

static void onLogMessage(const Worker_LogMessageOp *op, bool *disconnected);
static void onAddComponent(ClientContext *context, const Worker_AddComponentOp *op);
static void onAuthorityChange(ClientContext *context, const Worker_AuthorityChangeOp *op);
static void onUpdateComponent(ClientContext *context, const Worker_ComponentUpdateOp *op);
static void updateGame(ShovelerGame *game, double dt);
static void updateAuthoritativeViewComponentFunction(ShovelerGame *game, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value, void *clientContextPointer);
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

	ShovelerGameCameraSettings cameraSettings;
	cameraSettings.frame = clientConfiguration.controllerSettings.frame;
	cameraSettings.projection.fieldOfViewY = 2.0f * SHOVELER_PI * 50.0f / 360.0f;
	cameraSettings.projection.aspectRatio = (float) windowSettings.windowedWidth / windowSettings.windowedHeight;
	cameraSettings.projection.nearClippingPlane = 0.01;
	cameraSettings.projection.farClippingPlane = 1000;

	ClientContext context;
	context.connection = connection;
	context.clientConfiguration = &clientConfiguration;
	context.disconnected = false;
	context.clientPingTickCallback = NULL;

	ShovelerGame *game = shovelerGameCreate(updateGame, updateAuthoritativeViewComponentFunction, &context, &windowSettings, &cameraSettings, &clientConfiguration.controllerSettings);
	if(game == NULL) {
		return EXIT_FAILURE;
	}
	context.game = game;
	ShovelerView *view = game->view;
	shovelerClientRegisterViewComponentTypes(view);

	game->controller->lockMoveX = clientConfiguration.controllerLockMoveX;
	game->controller->lockMoveY = clientConfiguration.controllerLockMoveY;
	game->controller->lockMoveZ = clientConfiguration.controllerLockMoveZ;
	game->controller->lockTiltX = clientConfiguration.controllerLockTiltX;
	game->controller->lockTiltY = clientConfiguration.controllerLockTiltY;

	ShovelerResources *resources = shovelerResourcesCreate(/* TODO: on demand resource loading */ NULL, NULL);
	shovelerResourcesImagePngRegister(resources);

	while(shovelerGameIsRunning(game) && !context.disconnected) {
		Worker_OpList *opList = Worker_Connection_GetOpList(connection, 0);
		for(size_t i = 0; i < opList->op_count; ++i) {
			Worker_Op *op = &opList->ops[i];
			switch(op->op_type) {
				case WORKER_OP_TYPE_DISCONNECT:
					shovelerLogInfo("Disconnected from SpatialOS with code %d: %s", op->op.disconnect.reason, op->op.disconnect.connection_status_code);
					context.disconnected = true;
					break;
				case WORKER_OP_TYPE_FLAG_UPDATE:
					shovelerLogTrace("WORKER_OP_TYPE_FLAG_UPDATE");
					break;
				case WORKER_OP_TYPE_LOG_MESSAGE:
					onLogMessage(&op->op.log_message, &context.disconnected);
					break;
				case WORKER_OP_TYPE_METRICS:
					Worker_Connection_SendMetrics(connection, &op->op.metrics.metrics);
					break;
				case WORKER_OP_TYPE_CRITICAL_SECTION:
					shovelerLogTrace("WORKER_OP_TYPE_CRITICAL_SECTION");
					break;
				case WORKER_OP_TYPE_ADD_ENTITY:
					shovelerViewAddEntity(view, op->op.add_entity.entity_id);
					break;
				case WORKER_OP_TYPE_REMOVE_ENTITY:
					shovelerViewRemoveEntity(view, op->op.add_entity.entity_id);
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
					onAddComponent(&context, &op->op.add_component);
					break;
				case WORKER_OP_TYPE_REMOVE_COMPONENT:
					shovelerLogInfo("WORKER_OP_TYPE_REMOVE_COMPONENT");
					break;
				case WORKER_OP_TYPE_AUTHORITY_CHANGE:
					onAuthorityChange(&context, &op->op.authority_change);
					break;
				case WORKER_OP_TYPE_COMPONENT_UPDATE:
					onUpdateComponent(&context, &op->op.component_update);
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

static void onLogMessage(const Worker_LogMessageOp *op, bool *disconnected)
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

static void onAddComponent(ClientContext *context, const Worker_AddComponentOp *op)
{
	ShovelerView *view = context->game->view;

	const char *componentTypeId = shovelerClientResolveComponentTypeId((int) op->data.component_id);
	if(componentTypeId == NULL) {
		shovelerLogWarning("Received add for unknown component ID %d on entity %lld, ignoring.", op->data.component_id, op->entity_id);
		return;
	}

	ShovelerViewEntity *entity = shovelerViewGetEntity(view, op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning("Received add component %d (%s) for unknown entity %lld, ignoring.", op->data.component_id, componentTypeId, op->entity_id);
		return;
	}

	ShovelerComponent *component = shovelerViewEntityAddComponent(entity, componentTypeId);
	if(component == NULL) {
		shovelerLogWarning("Failed to add component %d (%s) to entity %lld.", op->data.component_id, componentTypeId, op->entity_id);
		return;
	}

	shovelerLogInfo("Adding entity %lld component %d (%s).", op->entity_id, op->data.component_id, componentTypeId);
	shovelerClientApplyComponentData(
		view,
		component,
		op->data.schema_type,
		context->clientConfiguration->positionMappingX,
		context->clientConfiguration->positionMappingY,
		context->clientConfiguration->positionMappingZ);

	shovelerComponentActivate(component);
}

static void onAuthorityChange(ClientContext *context, const Worker_AuthorityChangeOp *op)
{
	const char *componentTypeId = shovelerClientResolveComponentTypeId((int) op->component_id);
	if(componentTypeId == NULL) {
		shovelerLogWarning("Received authority change for unknown component ID %d on entity %lld, ignoring.", op->component_id, op->entity_id);
		return;
	}

	ShovelerViewEntity *entity = shovelerViewGetEntity(context->game->view, op->entity_id);
	if (entity == NULL) {
		shovelerLogWarning("Received authority change for unknown entity %lld, ignoring.", op->entity_id);
		return;
	}

	if(op->authority == WORKER_AUTHORITY_AUTHORITATIVE) {
		shovelerLogInfo("Gained authority over entity %lld component %d (%s).", op->entity_id, op->component_id, componentTypeId);
		shovelerViewEntityDelegate(entity, componentTypeId);

		if(op->component_id == clientComponentId) {
			shovelerLogInfo("Gained client authority over entity %lld.", op->entity_id);
			context->clientEntityId = op->entity_id;
			context->clientPingTickCallback = shovelerExecutorSchedulePeriodic(context->game->updateExecutor, 0, clientPingTimeoutMs, clientPingTick, context);
		}
	} else if(op->authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE) {
		shovelerLogInfo("Lost authority over entity %lld component %d (%s).", op->entity_id, op->component_id, componentTypeId);
		shovelerViewEntityUndelegate(entity, componentTypeId);

		if(op->component_id == clientComponentId) {
			shovelerLogWarning("Lost client authority over entity %lld.", op->entity_id);
			context->clientEntityId = 0;
			shovelerExecutorRemoveCallback(context->game->updateExecutor, context->clientPingTickCallback);
		}
	}
}

static void onUpdateComponent(ClientContext *context, const Worker_ComponentUpdateOp *op)
{
	ShovelerView *view = context->game->view;

	const char *componentTypeId = shovelerClientResolveComponentTypeId((int) op->update.component_id);
	if(componentTypeId == NULL) {
		shovelerLogWarning("Received update for unknown component ID %d on entity %lld, ignoring.", op->update.component_id, op->entity_id);
		return;
	}

	ShovelerViewEntity *entity = shovelerViewGetEntity(view, op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning("Received update component %d (%s) for unknown entity %lld, ignoring.", op->update.component_id, componentTypeId, op->entity_id);
		return;
	}

	ShovelerComponent *component = shovelerViewEntityGetComponent(entity, componentTypeId);
	if(component == NULL) {
		shovelerLogWarning("Received update for non-existing component %d (%s) on unknown entity %lld, ignoring.", op->update.component_id, componentTypeId, op->entity_id);
		return;
	}

	shovelerLogInfo("Updating entity %lld component %d (%s).", op->entity_id, op->update.component_id, componentTypeId);
	shovelerClientApplyComponentUpdate(
		view,
		component,
		op->update.schema_type,
		context->clientConfiguration->positionMappingX,
		context->clientConfiguration->positionMappingY,
		context->clientConfiguration->positionMappingZ);

	shovelerComponentActivate(component);
}

static void updateGame(ShovelerGame *game, double dt)
{
	shovelerCameraUpdateView(game->camera);
}

static void updateAuthoritativeViewComponentFunction(ShovelerGame *game, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value, void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;
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
