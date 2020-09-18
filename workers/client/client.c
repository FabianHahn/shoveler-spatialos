#include <stdlib.h> // srand
#include <string.h> // memset
#include <time.h> // time

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <shoveler/camera/perspective.h>
#include <shoveler/constants.h>
#include <shoveler/component.h>
#include <shoveler/component/client.h>
#include <shoveler/component/position.h>
#include <shoveler/connect.h>
#include <shoveler/game.h>
#include <shoveler/global.h>
#include <shoveler/log.h>
#include <shoveler/resources/image_png.h>
#include <shoveler/resources.h>
#include <shoveler/types.h>
#include <shoveler/view.h>

#include "configuration.h"
#include "interest.h"
#include "schema.h"

typedef struct {
	Worker_Connection *connection;
	ShovelerGame *game;
	ShovelerClientConfiguration *clientConfiguration;
	long long int clientEntityId;
	bool disconnected;
	ShovelerExecutorCallback *clientPingTickCallback;
	bool clientInterestAuthoritative;
	bool absoluteInterest;
	bool restrictController;
	bool viewDependenciesUpdated;
	double lastInterestUpdatePositionY;
	double edgeLength;
	ShovelerVector3 lastImprobablePosition;
	bool improbablePositionAuthoritative;
	int64_t lastHeartbeatPongTime;
	double meanHeartbeatLatencyMs;
	double meanTimeSinceLastHeartbeatPongMs;
} ClientContext;

static const long long int bootstrapEntityId = 1;
static const long long int bootstrapComponentId = 1334;
static const long long int clientComponentId = 1335;
static const int64_t clientPingTimeoutMs = 999;
static const int64_t clientStatusTimeoutMs = 2449;
static const float improbablePositionUpdateDistance = 1.0f;
static const double meanHeartbeatMovingExponentialFactor = 0.5f;
static const double meanTimeSinceLastHeartbeatPongExponentialFactor = 0.05f;

static void onLogMessage(void *user_data, const Worker_LogData *message);
static void onAddComponent(ClientContext *context, const Worker_AddComponentOp *op);
static void onAuthorityChange(ClientContext *context, const Worker_AuthorityChangeOp *op);
static void onUpdateComponent(ClientContext *context, const Worker_ComponentUpdateOp *op);
static void onRemoveComponent(ClientContext *context, const Worker_RemoveComponentOp *op);
static void updateGame(ShovelerGame *game, double dt);
static void updateAuthoritativeViewComponentFunction(ShovelerGame *game, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value, void *clientContextPointer);
static void clientPingTick(void *clientContextPointer);
static void clientStatus(void *clientContextPointer);
static void mouseButtonEvent(ShovelerInput *input, int button, int action, int mods, void *clientContextPointer);
static void viewDependencyCallbackFunction(ShovelerView *view, const ShovelerViewQualifiedComponent *dependencySource, const ShovelerViewQualifiedComponent *dependencyTarget, bool added, void *contextPointer);
static void updateInterest(ClientContext *context, bool absoluteInterest, ShovelerVector3 position, double edgeLength);
static void updateEdgeLength(ClientContext *context, ShovelerVector3 position);
static void keyHandler(ShovelerInput *input, int key, int scancode, int action, int mods, void *clientContextPointer);
static ShovelerVector3 getEntitySpatialOsPosition(ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ, long long int entityId);

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

	Worker_LogsinkParameters logsink;
	logsink.logsink_type = WORKER_LOGSINK_TYPE_CALLBACK;
	logsink.filter_parameters.categories = WORKER_LOG_CATEGORY_NETWORK_STATUS | WORKER_LOG_CATEGORY_LOGIN;
	logsink.filter_parameters.level = WORKER_LOG_LEVEL_INFO;
	logsink.filter_parameters.callback = NULL;
	logsink.filter_parameters.user_data = NULL;
	logsink.log_callback_parameters.log_callback = onLogMessage;
	logsink.log_callback_parameters.user_data = NULL;

	Worker_ConnectionParameters connectionParameters = Worker_DefaultConnectionParameters();
	connectionParameters.worker_type = "ShovelerClient";
	connectionParameters.network.connection_type = WORKER_NETWORK_CONNECTION_TYPE_MODULAR_KCP;
	connectionParameters.network.modular_kcp.security_type = WORKER_NETWORK_SECURITY_TYPE_DTLS;
	connectionParameters.default_component_vtable = &componentVtable;
	connectionParameters.logsink_count = 1;
	connectionParameters.logsinks = &logsink;
	connectionParameters.enable_logging_at_startup = true;

	shovelerLogInfo("Using SpatialOS C Worker SDK '%s'.", Worker_ApiVersionStr());
	Worker_Connection *connection = shovelerWorkerConnect(argc, argv, /* argumentOffset */ 0, &connectionParameters);
	assert(connection != NULL);
	if(!Worker_Connection_IsConnected(connection)) {
		shovelerLogError("Failed to connect to SpatialOS deployment: %s", Worker_Connection_GetConnectionStatusDetailString(connection));
		Worker_Connection_Destroy(connection);
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
	shovelerLogTrace("Sent create entity command request %lld.", createClientEntityCommandRequestId);

	ShovelerClientConfiguration clientConfiguration;
	if(!shovelerClientGetWorkerConfiguration(connection, &clientConfiguration)) {
		shovelerLogError("Failed to retrieve client configuration.");
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
	context.clientEntityId = 0;
	context.disconnected = false;
	context.clientPingTickCallback = NULL;
	context.clientInterestAuthoritative = false;
	context.absoluteInterest = false;
	context.restrictController = true;
	context.viewDependenciesUpdated = false;
	context.lastInterestUpdatePositionY = 0.0f;
	context.edgeLength = 20.5f;
	context.lastImprobablePosition = shovelerVector3(0.0f, 0.0f, 0.0f);
	context.improbablePositionAuthoritative = false;
	context.lastHeartbeatPongTime = g_get_monotonic_time();
	context.meanHeartbeatLatencyMs = 0.0;
	context.meanTimeSinceLastHeartbeatPongMs = 0.5 * (double) clientPingTimeoutMs;

	ShovelerGame *game = shovelerGameCreate(updateGame, updateAuthoritativeViewComponentFunction, &context, &windowSettings, &cameraSettings, &clientConfiguration.controllerSettings);
	if(game == NULL) {
		return EXIT_FAILURE;
	}
	context.game = game;
	ShovelerView *view = game->view;
	shovelerClientRegisterViewComponentTypes(view);
	shovelerInputAddKeyCallback(game->input, keyHandler, &context);
	shovelerInputAddMouseButtonCallback(game->input, mouseButtonEvent, &context);
	ShovelerExecutorCallback *clientStatusCallback = shovelerExecutorSchedulePeriodic(game->updateExecutor, 0, clientStatusTimeoutMs, clientStatus, &context);

	game->controller->lockMoveX = clientConfiguration.controllerLockMoveX;
	game->controller->lockMoveY = clientConfiguration.controllerLockMoveY;
	game->controller->lockMoveZ = clientConfiguration.controllerLockMoveZ;
	game->controller->lockTiltX = clientConfiguration.controllerLockTiltX;
	game->controller->lockTiltY = clientConfiguration.controllerLockTiltY;

	ShovelerResources *resources = shovelerResourcesCreate(/* TODO: on demand resource loading */ NULL, NULL);
	shovelerResourcesImagePngRegister(resources);

	shovelerViewAddDependencyCallback(view, viewDependencyCallbackFunction, &context);

	while(shovelerGameIsRunning(game) && !context.disconnected) {
		context.viewDependenciesUpdated = false;

		Worker_OpList *opList = Worker_Connection_GetOpList(connection, 0);
		for(size_t i = 0; i < opList->op_count; ++i) {
			Worker_Op *op = &opList->ops[i];
			switch(op->op_type) {
				case WORKER_OP_TYPE_DISCONNECT:
					shovelerLogInfo("Disconnected from SpatialOS with code %d: %s", op->op.disconnect.connection_status_code, op->op.disconnect.reason);
					context.disconnected = true;
					break;
				case WORKER_OP_TYPE_FLAG_UPDATE:
					shovelerLogTrace("WORKER_OP_TYPE_FLAG_UPDATE");
					break;
				case WORKER_OP_TYPE_LOG_MESSAGE:
					// deprecated and can be ignored, we receive all log messages through logsink already.
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
					shovelerLogTrace("WORKER_OP_TYPE_RESERVE_ENTITY_IDS_RESPONSE");
					break;
				case WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE:
					shovelerLogTrace("WORKER_OP_TYPE_CREATE_ENTITY_RESPONSE");
					break;
				case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
					shovelerLogTrace("WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE");
					break;
				case WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE:
					shovelerLogTrace("WORKER_OP_TYPE_ENTITY_QUERY_RESPONSE");
					break;
				case WORKER_OP_TYPE_ADD_COMPONENT:
					onAddComponent(&context, &op->op.add_component);
					break;
				case WORKER_OP_TYPE_REMOVE_COMPONENT:
					onRemoveComponent(&context, &op->op.remove_component);
					break;
				case WORKER_OP_TYPE_AUTHORITY_CHANGE:
					onAuthorityChange(&context, &op->op.authority_change);
					break;
				case WORKER_OP_TYPE_COMPONENT_UPDATE:
					onUpdateComponent(&context, &op->op.component_update);
					break;
				case WORKER_OP_TYPE_COMMAND_REQUEST:
					shovelerLogTrace("WORKER_OP_TYPE_COMMAND_REQUEST");
					break;
				case WORKER_OP_TYPE_COMMAND_RESPONSE:
					if (op->op.command_response.request_id == createClientEntityCommandRequestId) {
						shovelerLogTrace(
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

		ShovelerVector3 position = getEntitySpatialOsPosition(view, clientConfiguration.positionMappingX, clientConfiguration.positionMappingY, clientConfiguration.positionMappingZ, context.clientEntityId);
		updateEdgeLength(&context, position);

		if(context.clientInterestAuthoritative && context.viewDependenciesUpdated) {
			updateInterest(&context, context.absoluteInterest, position, context.edgeLength);
		}
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	Worker_Connection_Destroy(connection);

	shovelerExecutorRemoveCallback(game->updateExecutor, clientStatusCallback);
	shovelerGameFree(game);
	shovelerResourcesFree(resources);
	shovelerGlobalUninit();
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}

static void onLogMessage(void *user_data, const Worker_LogData *message)
{
	switch(message->log_level) {
		case WORKER_LOG_LEVEL_DEBUG:
			shovelerLogTrace("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_INFO:
			shovelerLogInfo("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_WARN:
			shovelerLogWarning("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_ERROR:
			shovelerLogError("[Worker SDK] %s", message->content);
			break;
		case WORKER_LOG_LEVEL_FATAL:
			shovelerLogError("[Worker SDK] [FATAL] %s", message->content);
			break;
	}
}

static void onAddComponent(ClientContext *context, const Worker_AddComponentOp *op)
{
	ShovelerView *view = context->game->view;

	ShovelerViewEntity *entity = shovelerViewGetEntity(view, op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning("Received add component %d for unknown entity %lld, ignoring.", op->data.component_id, op->entity_id);
		return;
	}

	const char *specialComponentType = shovelerClientResolveSpecialComponentId((int) op->data.component_id);
	if (specialComponentType != NULL) {
		// special case Metadata
		if (op->data.component_id == 53) {
			Schema_Object *fields = Schema_GetComponentDataFields(op->data.schema_type);
			uint32_t entityTypeLength = Schema_GetBytesLength(fields, /* fieldId */ 1);
			const uint8_t *entityTypeBytes = Schema_GetBytes(fields, /* fieldId */ 1);

			GString *entityType = g_string_new("");
			g_string_append_len(entityType, (const char *) entityTypeBytes, entityTypeLength);
			shovelerViewEntitySetType(entity, entityType->str);

			shovelerLogTrace("Added Metadata component to entity %lld, setting its type to '%s'.", op->entity_id, entityType->str);
			g_string_free(entityType, /* freeSegment */ true);

			return;
		}
		// special case improbable position for client entity
		if (op->data.component_id == 54 && op->entity_id == context->clientEntityId) {
			Schema_Object *fields = Schema_GetComponentDataFields(op->data.schema_type);
			Schema_Object *coordinatesField = Schema_GetObject(fields, /* fieldId */ 1);

			double coordinatesX = Schema_GetDouble(coordinatesField, /* fieldId */ 1);
			double coordinatesY = Schema_GetDouble(coordinatesField, /* fieldId */ 2);
			double coordinatesZ = Schema_GetDouble(coordinatesField, /* fieldId */ 3);

			ShovelerVector3 coordinates = shovelerVector3((float) coordinatesX, (float) coordinatesY, (float) coordinatesZ);

			float mappedCoordinatesX = shovelerCoordinateMap(coordinates, context->clientConfiguration->positionMappingX);
			float mappedCoordinatesY = shovelerCoordinateMap(coordinates, context->clientConfiguration->positionMappingY);
			float mappedCoordinatesZ = shovelerCoordinateMap(coordinates, context->clientConfiguration->positionMappingZ);
			context->lastImprobablePosition = shovelerVector3(mappedCoordinatesX, mappedCoordinatesY, mappedCoordinatesZ);

			shovelerLogTrace("Added Improbable Position component to client entity %lld with mapped coordinates (%.2f, %.2f, %.2f).", op->entity_id, mappedCoordinatesX, mappedCoordinatesY, mappedCoordinatesZ);

			return;
		}

		shovelerLogTrace("Added special component %s to entity %lld.", specialComponentType, op->entity_id);
		return;
	}

	const char *componentTypeId = shovelerClientResolveComponentTypeId((int) op->data.component_id);
	if(componentTypeId == NULL) {
		shovelerLogWarning("Received add for unknown component ID %d on entity %lld, ignoring.", op->data.component_id, op->entity_id);
		return;
	}

	ShovelerComponent *component = shovelerViewEntityAddComponent(entity, componentTypeId);
	if(component == NULL) {
		shovelerLogWarning("Failed to add component %d (%s) to entity %lld.", op->data.component_id, componentTypeId, op->entity_id);
		return;
	}

	shovelerLogTrace("Adding entity %lld component %d (%s).", op->entity_id, op->data.component_id, componentTypeId);
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
	// special case interest
	if (op->component_id == 58) {
		if(op->authority == WORKER_AUTHORITY_AUTHORITATIVE) {
			shovelerLogTrace("Received authority over interest component of client entity %lld.", op->entity_id);
			context->clientInterestAuthoritative = true;
			context->viewDependenciesUpdated = true;
		} else {
			shovelerLogWarning("Lost interest authority over client entity %lld.", op->entity_id);
			context->clientInterestAuthoritative = false;
		}
		return;
	}
	// special case improbable position
	if (op->component_id == 54) {
		if(op->authority == WORKER_AUTHORITY_AUTHORITATIVE) {
			shovelerLogTrace("Received authority over Improbable position component of client entity %lld.", op->entity_id);
			context->improbablePositionAuthoritative = true;
		} else {
			shovelerLogWarning("Lost Improbable position authority over client entity %lld.", op->entity_id);
			context->improbablePositionAuthoritative = false;
		}
		return;
	}

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
		shovelerLogTrace("Gained authority over entity %lld component %d (%s).", op->entity_id, op->component_id, componentTypeId);
		shovelerViewEntityDelegate(entity, componentTypeId);

		// If the component exists, we might be able to activate it now.
		ShovelerComponent *component = shovelerViewEntityGetComponent(entity, componentTypeId);
		if(component != NULL) {
			shovelerComponentActivate(component);
		}

		if(op->component_id == clientComponentId) {
			shovelerLogTrace("Gained client authority over entity %lld.", op->entity_id);
			context->clientEntityId = op->entity_id;
			context->clientPingTickCallback = shovelerExecutorSchedulePeriodic(context->game->updateExecutor, 0, clientPingTimeoutMs, clientPingTick, context);
		}
	} else if(op->authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE) {
		shovelerLogTrace("Lost authority over entity %lld component %d (%s).", op->entity_id, op->component_id, componentTypeId);
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

	ShovelerViewEntity *entity = shovelerViewGetEntity(view, op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning("Received update component %d for unknown entity %lld, ignoring.", op->update.component_id, op->entity_id);
		return;
	}

	const char *specialComponentType = shovelerClientResolveSpecialComponentId((int) op->update.component_id);
	if(specialComponentType != NULL) {
		// special case Metadata
		if(op->update.component_id == 53) {
			Schema_Object *fields = Schema_GetComponentUpdateFields(op->update.schema_type);
			uint32_t entityTypeLength = Schema_GetBytesLength(fields, /* fieldId */ 1);
			const uint8_t *entityTypeBytes = Schema_GetBytes(fields, /* fieldId */ 1);

			GString *entityType = g_string_new("");
			g_string_append_len(entityType, (const char *) entityTypeBytes, entityTypeLength);
			shovelerViewEntitySetType(entity, entityType->str);

			shovelerLogTrace("Updated Metadata component on entity %lld, setting its type to '%s'.", op->entity_id, entityType->str);
			g_string_free(entityType, /* freeSegment */ true);

			return;
		}

		// special case ClientHeartbeatPong
		if(op->update.component_id == 13352) {
			if(op->entity_id != context->clientEntityId) {
				shovelerLogWarning("Received ClientHeartbeatPong update for entity %lld that isn't the client entity %lld, which points to a broken interest setup", op->entity_id, context->clientEntityId);
				return;
			}

			Schema_Object *fields = Schema_GetComponentUpdateFields(op->update.schema_type);
			int64_t lastPing = Schema_GetInt64(fields, /* fieldId */ 1);

			context->lastHeartbeatPongTime = g_get_monotonic_time();
			context->meanHeartbeatLatencyMs *= (1.0 - meanHeartbeatMovingExponentialFactor);
			context->meanHeartbeatLatencyMs += meanHeartbeatMovingExponentialFactor * 0.001 * (double) (context->lastHeartbeatPongTime - lastPing);
		}

		shovelerLogTrace("Updated special component %s on entity %lld.", specialComponentType, op->entity_id);
		return;
	}

	const char *componentTypeId = shovelerClientResolveComponentTypeId((int) op->update.component_id);
	if(componentTypeId == NULL) {
		shovelerLogWarning("Received update for unknown component ID %d on entity %lld, ignoring.", op->update.component_id, op->entity_id);
		return;
	}

	ShovelerComponent *component = shovelerViewEntityGetComponent(entity, componentTypeId);
	if(component == NULL) {
		shovelerLogWarning("Received update for non-existing component %d (%s) on entity %lld, ignoring.", op->update.component_id, componentTypeId, op->entity_id);
		return;
	}

	shovelerLogTrace("Updating entity %lld component %d (%s).", op->entity_id, op->update.component_id, componentTypeId);
	shovelerClientApplyComponentUpdate(
		view,
		component,
		op->update.schema_type,
		context->clientConfiguration->positionMappingX,
		context->clientConfiguration->positionMappingY,
		context->clientConfiguration->positionMappingZ);

	shovelerComponentActivate(component);
}

static void onRemoveComponent(ClientContext *context, const Worker_RemoveComponentOp *op)
{
	ShovelerView *view = context->game->view;

	ShovelerViewEntity *entity = shovelerViewGetEntity(view, op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning("Received remove component %d for unknown entity %lld, ignoring.", op->component_id, op->entity_id);
		return;
	}

	const char *specialComponentType = shovelerClientResolveSpecialComponentId((int) op->component_id);
	if(specialComponentType != NULL) {
		shovelerLogTrace("Removed special component %s from entity %lld.", specialComponentType, op->entity_id);
		return;
	}

	const char *componentTypeId = shovelerClientResolveComponentTypeId((int) op->component_id);
	if(componentTypeId == NULL) {
		shovelerLogWarning("Received remove for unknown component ID %d on entity %lld, ignoring.", op->component_id, op->entity_id);
		return;
	}

	ShovelerComponent *component = shovelerViewEntityGetComponent(entity, componentTypeId);
	if(component == NULL) {
		shovelerLogWarning("Received remove for non-existing component %d (%s) on entity %lld, ignoring.", op->component_id, componentTypeId, op->entity_id);
		return;
	}

	shovelerLogTrace("Removing entity %lld component %d (%s).", op->entity_id, op->component_id, componentTypeId);
	shovelerViewEntityRemoveComponent(entity, componentTypeId);
}

static void updateGame(ShovelerGame *game, double dt)
{
	shovelerCameraUpdateView(game->camera);
}

static void updateAuthoritativeViewComponentFunction(ShovelerGame *game, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value, void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;

	{
		Schema_ComponentUpdate *componentUpdate = shovelerClientCreateComponentUpdate(component, configurationOption, value);

		Worker_ComponentUpdate update;
		update.component_id = shovelerClientResolveComponentSchemaId(component->type->id);
		update.schema_type = componentUpdate;

		Worker_UpdateParameters updateParameters;
		updateParameters.loopback = WORKER_COMPONENT_UPDATE_LOOPBACK_NONE;

		Worker_Connection_SendComponentUpdate(context->connection, component->entityId, &update, &updateParameters);
	}

	// check if we also need to update our Improbable position
	if(component->type->id == shovelerComponentTypeIdPosition && context->improbablePositionAuthoritative) {
		const ShovelerVector3 *position = shovelerComponentGetPosition(component);
		ShovelerVector3 difference = shovelerVector3LinearCombination(1.0f, *position, -1.0f, context->lastImprobablePosition);
		float distance2 = shovelerVector3Dot(difference, difference);

		if(distance2 > improbablePositionUpdateDistance * improbablePositionUpdateDistance) {
			Schema_ComponentUpdate *componentUpdate = shovelerClientCreateImprobablePositionUpdate(*position, context->clientConfiguration->positionMappingX, context->clientConfiguration->positionMappingY, context->clientConfiguration->positionMappingZ);

			Worker_ComponentUpdate update;
			update.component_id = 54;
			update.schema_type = componentUpdate;

			Worker_UpdateParameters updateParameters;
			updateParameters.loopback = WORKER_COMPONENT_UPDATE_LOOPBACK_NONE;

			Worker_Connection_SendComponentUpdate(context->connection, component->entityId, &update, &updateParameters);

			context->lastImprobablePosition = *position;
		}
	}
}

static void clientPingTick(void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;

	Schema_ComponentUpdate *componentUpdate = Schema_CreateComponentUpdate();
	Schema_Object *fields = Schema_GetComponentUpdateFields(componentUpdate);
	Schema_AddInt64(fields, /* fieldId */ 1, g_get_monotonic_time());

	Worker_ComponentUpdate update;
	update.component_id = 13351;
	update.schema_type = componentUpdate;

	Worker_UpdateParameters updateParameters;
	updateParameters.loopback = WORKER_COMPONENT_UPDATE_LOOPBACK_NONE;

	Worker_Connection_SendComponentUpdate(context->connection, context->clientEntityId, &update, &updateParameters);
	shovelerLogTrace("Sent client heartbeat ping update.");
}

static void clientStatus(void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;

	context->meanTimeSinceLastHeartbeatPongMs *= (1.0 - meanTimeSinceLastHeartbeatPongExponentialFactor);
	context->meanTimeSinceLastHeartbeatPongMs += meanTimeSinceLastHeartbeatPongExponentialFactor * 0.001 * (double) (g_get_monotonic_time() - context->lastHeartbeatPongTime);

	shovelerLogInfo(
		"Latency: %.0fms\t\tDesync: %.0fms",
		context->meanHeartbeatLatencyMs,
		fabs(context->meanTimeSinceLastHeartbeatPongMs - 0.5 * (double) clientPingTimeoutMs));
}

static void mouseButtonEvent(ShovelerInput *input, int button, int action, int mods, void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;

	ShovelerViewEntity *clientEntity = shovelerViewGetEntity(context->game->view, context->clientEntityId);
	if(clientEntity == NULL) {
		return;
	}

	ShovelerComponent *positionComponent = shovelerViewEntityGetComponent(clientEntity, shovelerComponentTypeIdPosition);
	if (positionComponent == NULL) {
		return;
	}

	const ShovelerVector3 *coordinates = shovelerComponentGetPosition(positionComponent);

	if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		if(context->clientConfiguration->gameType == SHOVELER_CLIENT_GAME_TYPE_TILES) {
			shovelerLogInfo("Sending dig hole command...");

			Worker_CommandRequest digHoleCommandRequest;
			memset(&digHoleCommandRequest, 0, sizeof(Worker_CommandRequest));
			digHoleCommandRequest.component_id = bootstrapComponentId;
			digHoleCommandRequest.command_index = 3;
			digHoleCommandRequest.schema_type = Schema_CreateCommandRequest();

			Schema_Object *digHoleRequest = Schema_GetCommandRequestObject(digHoleCommandRequest.schema_type);
			Schema_AddEntityId(digHoleRequest, /* fieldId */ 1, context->clientEntityId);
			Schema_Object *position = Schema_AddObject(digHoleRequest, /* fieldId */ 2);
			Schema_AddFloat(position, /* fieldId */ 1, coordinates->values[0]);
			Schema_AddFloat(position, /* fieldId */ 2, coordinates->values[1]);
			Schema_AddFloat(position, /* fieldId */ 3, coordinates->values[2]);

			Worker_RequestId digHoleCommandRequestId = Worker_Connection_SendCommandRequest(
				context->connection,
				bootstrapEntityId,
				&digHoleCommandRequest,
				/* timeout_millis */ NULL,
				/* command_parameters */ NULL);
			if(digHoleCommandRequestId < 0) {
				shovelerLogWarning("Failed to send dig hole command.");
			}
			shovelerLogTrace("Sent dig hole command request %lld.", digHoleCommandRequestId);
		} else {
			shovelerLogInfo("Sending client cube spawn command...");

			ShovelerCameraPerspective *perspectiveCamera = (ShovelerCameraPerspective *) context->game->camera->data;

			Worker_CommandRequest spawnCubeCommandRequest;
			memset(&spawnCubeCommandRequest, 0, sizeof(Worker_CommandRequest));
			spawnCubeCommandRequest.component_id = bootstrapComponentId;
			spawnCubeCommandRequest.command_index = 2;
			spawnCubeCommandRequest.schema_type = Schema_CreateCommandRequest();

			Schema_Object *spawnCubeRequest = Schema_GetCommandRequestObject(spawnCubeCommandRequest.schema_type);
			Schema_AddEntityId(spawnCubeRequest, /* fieldId */ 1, context->clientEntityId);
			Schema_Object *position = Schema_AddObject(spawnCubeRequest, /* fieldId */ 2);
			Schema_AddFloat(position, /* fieldId */ 1, coordinates->values[0]);
			Schema_AddFloat(position, /* fieldId */ 2, coordinates->values[1]);
			Schema_AddFloat(position, /* fieldId */ 3, coordinates->values[2]);
			Schema_Object *direction = Schema_AddObject(spawnCubeRequest, /* fieldId */ 3);
			Schema_AddFloat(direction, /* fieldId */ 1, perspectiveCamera->direction.values[0]);
			Schema_AddFloat(direction, /* fieldId */ 2, perspectiveCamera->direction.values[1]);
			Schema_AddFloat(direction, /* fieldId */ 3, perspectiveCamera->direction.values[2]);
			Schema_Object *rotation = Schema_AddObject(spawnCubeRequest, /* fieldId */ 4);
			Schema_AddFloat(rotation, /* fieldId */ 1, 0.0f);
			Schema_AddFloat(rotation, /* fieldId */ 2, 0.0f);
			Schema_AddFloat(rotation, /* fieldId */ 3, 0.0f);

			Worker_RequestId spawnCubeCommandRequestId = Worker_Connection_SendCommandRequest(
				context->connection,
				bootstrapEntityId,
				&spawnCubeCommandRequest,
				/* timeout_millis */ NULL,
				/* command_parameters */ NULL);
			if(spawnCubeCommandRequestId < 0) {
				shovelerLogWarning("Failed to send spawn cube command.");
			}
			shovelerLogTrace("Sent spawn cube command request %lld.", spawnCubeCommandRequestId);
		}
	}
}

static void viewDependencyCallbackFunction(ShovelerView *view, const ShovelerViewQualifiedComponent *dependencySource, const ShovelerViewQualifiedComponent *dependencyTarget, bool added, void *contextPointer) {
	ClientContext *context = (ClientContext *) contextPointer;
	context->viewDependenciesUpdated = true;
}

static void updateInterest(ClientContext *context, bool absoluteInterest, ShovelerVector3 position, double edgeLength)
{
	Schema_ComponentUpdate *componentUpdate = Schema_CreateComponentUpdate();
	Schema_Object *interest = Schema_GetComponentUpdateFields(componentUpdate);
	Schema_Object *interestEntry = Schema_AddObject(interest, /* fieldId */ 1);
	Schema_AddUint32(interestEntry, /* fieldId */ 1, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdClient));
	Schema_Object *componentInterest = Schema_AddObject(interestEntry, /* fieldId */ 2);

	int numQueries = shovelerClientComputeViewInterest(context->game->view, context->clientEntityId, absoluteInterest, position, edgeLength, componentInterest);

	Worker_ComponentUpdate update;
	update.component_id = 58;
	update.schema_type = componentUpdate;

	Worker_UpdateParameters updateParameters;
	updateParameters.loopback = WORKER_COMPONENT_UPDATE_LOOPBACK_NONE;

	Worker_Connection_SendComponentUpdate(context->connection, context->clientEntityId, &update, &updateParameters);

	shovelerLogInfo("Sent interest update with %d queries.", numQueries);
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

static void keyHandler(ShovelerInput *input, int key, int scancode, int action, int mods, void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;

	if(key == GLFW_KEY_F7 && action == GLFW_PRESS) {
		shovelerLogInfo("F7 key pressed, changing to %s interest.", context->absoluteInterest ? "relative" : "absolute");
		context->absoluteInterest = !context->absoluteInterest;

		ShovelerVector3 position = getEntitySpatialOsPosition(context->game->view, context->clientConfiguration->positionMappingX, context->clientConfiguration->positionMappingY, context->clientConfiguration->positionMappingZ, context->clientEntityId);
		updateEdgeLength(context, position);
		updateInterest(context, context->absoluteInterest, position, context->edgeLength);
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

static ShovelerVector3 getEntitySpatialOsPosition(ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ, long long int entityId)
{
	ShovelerVector3 spatialOsPosition = shovelerVector3(0.0f, 0.0f, 0.0f);

	ShovelerViewEntity *entity = shovelerViewGetEntity(view, entityId);
	if(entity != NULL) {
		ShovelerComponent *component = shovelerViewEntityGetComponent(entity, shovelerComponentTypeIdPosition);
		if (component != NULL) {
			const ShovelerVector3 *coordinates = shovelerComponentGetPosition(component);

			// TODO: inverse mapping?
			spatialOsPosition = shovelerVector3(
				shovelerCoordinateMap(*coordinates, mappingX),
				shovelerCoordinateMap(*coordinates, mappingY),
				shovelerCoordinateMap(*coordinates, mappingZ));
		}
	}

	return spatialOsPosition;
}
