#include <assert.h>
#include <inttypes.h> // PRIu32 PRId64
#include <stdlib.h> // srand
#include <string.h> // memset
#include <time.h> // time

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <shoveler/constants.h>
#include <shoveler/connect.h>
#include <shoveler/log.h>
#include <shoveler/resources/image_png.h>
#include <shoveler/resources.h>
#include <shoveler/schema.h>
#include <shoveler/types.h>
#include <shoveler/worker_log.h>
#include <shoveler/view.h>
#include <shoveler/executor.h>
#include <shoveler/configuration.h>

typedef struct {
	GString *tilesetRows;
	GString *tilesetColumns;
	GString *tilesetIds;
} TilemapTiles;

typedef struct {
	uint32_t componentId;
	bool authoritative;
	union {
		ShovelerVector3 position;
		TilemapTiles tilemapTiles;
	};
} Component;

typedef struct {
	int64_t entityId;
	GHashTable *components;
} Entity;

typedef struct {
	Worker_Connection *connection;
	bool disconnected;
	ShovelerExecutor *executor;
	ShovelerExecutorCallback *clientPingTickCallback;
	GHashTable *entities;
	int64_t clientEntityId;
	enum {
		kUp,
		kDown,
		kLeft,
		kRight
	} direction;
	ShovelerVector3 lastImprobablePosition;
	int64_t lastHeartbeatPongTime;
	double meanHeartbeatLatencyMs;
	double meanTimeSinceLastHeartbeatPongMs;
} ClientContext;

static void onAddComponent(ClientContext *context, const Worker_AddComponentOp *op);
static void onComponentUpdate(ClientContext *context, const Worker_ComponentUpdateOp *op);
static void onAuthorityChange(ClientContext *context, const Worker_AuthorityChangeOp *op);
static void clientPingTick(void *clientContextPointer);
static void clientDirectionChange(void *clientContextPointer);
static void clientStatus(void *clientContextPointer);
static void move(ClientContext *context, Component *positionComponent, int dtMs);
static bool validatePosition(ClientContext *context, ShovelerVector3 coordinates);
static bool validatePoint(ClientContext *context, ShovelerVector3 coordinates);
static int64_t getChunkBackgroundEntityId(int chunkX, int chunkZ);
static TilemapTiles *getChunkBackgroundTiles(ClientContext *context, int64_t chunkBackgroundEntityId);
static ShovelerVector2 tileToWorld(int chunkX, int chunkZ, int tileX, int tileZ);
static void worldToTile(double x, double z, int *outputChunkX, int *outputChunkZ, int *outputTileX, int *outputTileZ);
static void freeEntity(void *entityPointer);
static void freeComponent(void *componentPointer);

static const long long int bootstrapEntityId = 1;
static const int64_t clientPingTimeoutMs = 999;
static const int64_t clientDirectionChangeTimeoutMs = 250;
static const int64_t clientStatusTimeoutMs = 2449;
static const float velocity = 1.5f;
static const int directionChangeChancePercent = 10;
static const int tickRateHz = 30;
static const int halfMapWidth = 100;
static const int halfMapHeight = 100;
static const int chunkSize = 10;
static const int64_t firstChunkEntityId = 12;
static const double characterSize = 0.9;
static const float improbablePositionUpdateDistance = 1.0f;
static const double meanHeartbeatMovingExponentialFactor = 0.5f;
static const double meanTimeSinceLastHeartbeatPongExponentialFactor = 0.05f;

int main(int argc, char **argv) {
	srand(time(NULL));

	shovelerLogInit("shoveler-spatialos/", SHOVELER_LOG_LEVEL_ALL, stdout);

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
	logsink.log_callback_parameters.log_callback = shovelerWorkerOnLogMessage;
	logsink.log_callback_parameters.user_data = NULL;

	Worker_ConnectionParameters connectionParameters = Worker_DefaultConnectionParameters();
	connectionParameters.worker_type = "ShovelerBotClient";
	connectionParameters.network.connection_type = WORKER_NETWORK_CONNECTION_TYPE_MODULAR_KCP;
	connectionParameters.network.modular_kcp.security_type = WORKER_NETWORK_SECURITY_TYPE_INSECURE;
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

	ClientContext context;
	context.connection = connection;
	context.disconnected = false;
	context.executor = shovelerExecutorCreateDirect();
	context.clientPingTickCallback = NULL;
	context.entities = g_hash_table_new_full(g_int64_hash, g_int64_equal, /* key_destroy_func */ NULL, freeEntity);
	context.clientEntityId = 0;
	context.clientEntityId = 0;
	context.direction = kUp;
	context.lastImprobablePosition = shovelerVector3(0.0f, 0.0f, 0.0f);
	context.clientEntityId = 0;
	context.lastHeartbeatPongTime = g_get_monotonic_time();
	context.meanHeartbeatLatencyMs = 0.0;
	context.meanTimeSinceLastHeartbeatPongMs = 0.5 * (double) clientPingTimeoutMs;

	shovelerExecutorSchedulePeriodic(context.executor, 0, clientDirectionChangeTimeoutMs, clientDirectionChange, &context);
	shovelerExecutorSchedulePeriodic(context.executor, 0, clientStatusTimeoutMs, clientStatus, &context);

	int minXFlag = 0;
	int minZFlag = 0;
	int sizeXFlag = 0;
	int sizeZFlag = 0;
	shovelerWorkerConfigurationParseIntFlag(context.connection, "starting_chunk_min_x", &minXFlag);
	shovelerWorkerConfigurationParseIntFlag(context.connection, "starting_chunk_min_z", &minZFlag);
	shovelerWorkerConfigurationParseIntFlag(context.connection, "starting_chunk_size_x", &sizeXFlag);
	shovelerWorkerConfigurationParseIntFlag(context.connection, "starting_chunk_size_z", &sizeZFlag);

	Worker_CommandRequest createClientEntityCommandRequest;
	memset(&createClientEntityCommandRequest, 0, sizeof(Worker_CommandRequest));
	createClientEntityCommandRequest.component_id = shovelerWorkerSchemaComponentIdBootstrap;
	createClientEntityCommandRequest.command_index = shovelerWorkerSchemaBootstrapCommandIdCreateClientEntity;
	createClientEntityCommandRequest.schema_type = Schema_CreateCommandRequest();
	Schema_Object *createClientEntityRequest = Schema_GetCommandRequestObject(createClientEntityCommandRequest.schema_type);

	if(minXFlag && minZFlag && sizeXFlag && sizeZFlag) {
		shovelerLogInfo("Overriding starting chunk region to min (%d, %d) and size (%d, %d).", minXFlag, minXFlag, sizeXFlag, sizeZFlag);
		Schema_Object *startingChunkRegion = Schema_AddObject(createClientEntityRequest, shovelerWorkerSchemaCreateClientEntityRequestFieldIdStartingChunkRegion);
		Schema_AddInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdMinX, minXFlag);
		Schema_AddInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdMaxX, minZFlag);
		Schema_AddInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdSizeX, sizeXFlag);
		Schema_AddInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdSizeZ, sizeZFlag);
	}

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

	gint64 lastTickTime = g_get_monotonic_time();
	while(!context.disconnected) {
		gint64 tickStartTime = g_get_monotonic_time();
		gint64 dtNs = tickStartTime - lastTickTime;
		gint64 remainingNs = (1000 * 1000 * 1000 / tickRateHz) - dtNs;
		if(remainingNs < 0) {
			remainingNs = 0;
		}

		Worker_OpList *opList = Worker_Connection_GetOpList(connection, remainingNs / 1000 / 1000);
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
				case WORKER_OP_TYPE_ADD_ENTITY: {
					Entity *entity = malloc(sizeof(Entity));
					entity->entityId = op->op.add_entity.entity_id;
					entity->components = g_hash_table_new_full(g_int_hash, g_int_equal, /* key_destroy_func */ NULL, freeComponent);

					g_hash_table_insert(context.entities, &entity->entityId, entity);
				} break;
				case WORKER_OP_TYPE_REMOVE_ENTITY:
					g_hash_table_remove(context.entities, &op->op.remove_entity.entity_id);
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
				case WORKER_OP_TYPE_REMOVE_COMPONENT: {
					Entity *entity = g_hash_table_lookup(context.entities, &op->op.remove_component.entity_id);
					if(entity == NULL) {
						shovelerLogWarning(
							"Received remove entity %"PRId64" component %"PRIu32" but entity is not in view, ignoring",
							op->op.remove_component.entity_id,
							op->op.remove_component.component_id);
						break;
					}

					g_hash_table_remove(entity->components, &op->op.remove_component.component_id);
				} break;
				case WORKER_OP_TYPE_AUTHORITY_CHANGE:
					onAuthorityChange(&context, &op->op.authority_change);
					break;
				case WORKER_OP_TYPE_COMPONENT_UPDATE:
					onComponentUpdate(&context, &op->op.component_update);
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

		shovelerExecutorUpdateNow(context.executor);

		if(context.clientEntityId != 0) {
			Entity *clientEntity = g_hash_table_lookup(context.entities, &context.clientEntityId);
			if(clientEntity != NULL) {
				uint32_t positionComponentId = shovelerWorkerSchemaComponentIdPosition;
				Component *positionComponent = g_hash_table_lookup(clientEntity->components, &positionComponentId);
				if(positionComponent) {
					if(positionComponent->authoritative) {
						move(&context, positionComponent, dtNs / 1000 / 1000);
					}
				}
			}
		}

		lastTickTime = tickStartTime;
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	Worker_Connection_Destroy(connection);
	shovelerExecutorFree(context.executor);
	g_hash_table_destroy(context.entities);
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}

static void onAddComponent(ClientContext *context, const Worker_AddComponentOp *op)
{
	Entity *entity = g_hash_table_lookup(context->entities, &op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning(
			"Received add entity %"PRId64" component %"PRIu32" but entity is not in view, ignoring",
			op->entity_id,
			op->data.component_id);
		return;
	}

	Component *component = malloc(sizeof(Component));
	memset(component, 0, sizeof(Component));
	component->componentId = op->data.component_id;
	component->authoritative = false;

	Schema_Object *fields = Schema_GetComponentDataFields(op->data.schema_type);

	if(component->componentId == shovelerWorkerSchemaComponentIdPosition) {
		Schema_Object *coordinates = Schema_GetObject(fields, shovelerWorkerSchemaPositionFieldIdCoordinates);
		if (coordinates == NULL) {
			shovelerLogWarning(
				"Received add entity %"PRId64" position component without coordinates.",
				op->entity_id);
			return;
		}

		component->position.values[0] = Schema_GetFloat(coordinates, shovelerWorkerSchemaVector3FieldIdX);
		component->position.values[1] = Schema_GetFloat(coordinates, shovelerWorkerSchemaVector3FieldIdY);
		component->position.values[2] = Schema_GetFloat(coordinates, shovelerWorkerSchemaVector3FieldIdZ);
	} else if(component->componentId == shovelerWorkerSchemaComponentIdTilemapTiles) {
		component->tilemapTiles.tilesetColumns = g_string_new("");
		component->tilemapTiles.tilesetRows = g_string_new("");
		component->tilemapTiles.tilesetIds = g_string_new("");

		uint32_t numTilesetColumns = Schema_GetBytesCount(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns);
		uint32_t numTilesetRows = Schema_GetBytesCount(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows);
		uint32_t numTilesetIds = Schema_GetBytesCount(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds);
		if (numTilesetColumns != 1 || numTilesetRows != 1 || numTilesetIds != 1) {
			shovelerLogWarning(
				"Received add entity %"PRId64" tilemap tiles component without tileset columns, rows or ids.",
				op->entity_id);
			return;
		}

		uint32_t tilesetColumnsLength = Schema_GetBytesLength(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns);
		uint32_t tilesetRowsLength = Schema_GetBytesLength(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows);
		uint32_t tilesetIdsLength = Schema_GetBytesLength(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds);

		const uint8_t *tilesetColumnsBytes = Schema_GetBytes(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns);
		const uint8_t *tilesetRowsBytes = Schema_GetBytes(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows);
		const uint8_t *tilesetIdsBytes = Schema_GetBytes(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds);

		g_string_append_len(component->tilemapTiles.tilesetColumns, (const char *) tilesetColumnsBytes, tilesetColumnsLength);
		g_string_append_len(component->tilemapTiles.tilesetRows, (const char *) tilesetRowsBytes, tilesetRowsLength);
		g_string_append_len(component->tilemapTiles.tilesetIds, (const char *) tilesetIdsBytes, tilesetIdsLength);
	}

	g_hash_table_insert(entity->components, &component->componentId, component);
}

static void onComponentUpdate(ClientContext *context, const Worker_ComponentUpdateOp *op)
{
	Entity *entity = g_hash_table_lookup(context->entities, &op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning(
			"Received update entity %"PRId64" component %"PRIu32" for entity not in view, ignoring.",
			op->entity_id,
			op->update.component_id);
		return;
	}

	Component *component = g_hash_table_lookup(entity->components, &op->update.component_id);
	if(component == NULL) {
		shovelerLogWarning(
			"Received update entity %"PRId64" component %"PRIu32" for component not in view, ignoring.",
			op->entity_id,
			op->update.component_id);
		return;
	}

	Schema_Object *fields = Schema_GetComponentUpdateFields(op->update.schema_type);

	if(op->update.component_id == shovelerWorkerSchemaComponentIdClientHeartbeatPong) {
		if(op->entity_id != context->clientEntityId) {
			shovelerLogWarning("Received ClientHeartbeatPong update for entity %lld that isn't the client entity %lld, which points to a broken interest setup", op->entity_id, context->clientEntityId);
			return;
		}

		int64_t lastPing = Schema_GetInt64(fields, shovelerWorkerSchemaClientHeartbeatPongFieldIdLastUpdatedTime);

		context->lastHeartbeatPongTime = g_get_monotonic_time();
		context->meanHeartbeatLatencyMs *= (1.0 - meanHeartbeatMovingExponentialFactor);
		context->meanHeartbeatLatencyMs += meanHeartbeatMovingExponentialFactor * 0.001 * (double) (context->lastHeartbeatPongTime - lastPing);
	} else if(component->componentId == shovelerWorkerSchemaComponentIdTilemapTiles) {
		Schema_Object *coordinates = Schema_GetObject(fields, shovelerWorkerSchemaPositionFieldIdCoordinates);
		if (coordinates == NULL) {
			shovelerLogWarning(
				"Received update entity %"PRId64" position component without coordinates.",
				op->entity_id);
			return;
		}

		component->position.values[0] = Schema_GetFloat(coordinates, shovelerWorkerSchemaVector3FieldIdX);
		component->position.values[1] = Schema_GetFloat(coordinates, shovelerWorkerSchemaVector3FieldIdY);
		component->position.values[2] = Schema_GetFloat(coordinates, shovelerWorkerSchemaVector3FieldIdZ);
	} else if(op->update.component_id == shovelerWorkerSchemaComponentIdTilemapTiles) {
		g_string_set_size(component->tilemapTiles.tilesetColumns, 0);
		g_string_set_size(component->tilemapTiles.tilesetRows, 0);
		g_string_set_size(component->tilemapTiles.tilesetIds, 0);

		uint32_t numTilesetColumns = Schema_GetBytesCount(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns);
		uint32_t numTilesetRows = Schema_GetBytesCount(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows);
		uint32_t numTilesetIds = Schema_GetBytesCount(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds);
		if (numTilesetColumns != 1 || numTilesetRows != 1 || numTilesetIds != 1) {
			shovelerLogWarning(
				"Received update entity %"PRId64" tilemap tiles component without tileset columns, rows or ids.",
				op->entity_id);
			return;
		}

		uint32_t tilesetColumnsLength = Schema_GetBytesLength(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns);
		uint32_t tilesetRowsLength = Schema_GetBytesLength(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows);
		uint32_t tilesetIdsLength = Schema_GetBytesLength(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds);

		const uint8_t *tilesetColumnsBytes = Schema_GetBytes(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns);
		const uint8_t *tilesetRowsBytes = Schema_GetBytes(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows);
		const uint8_t *tilesetIdsBytes = Schema_GetBytes(fields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds);

		g_string_append_len(component->tilemapTiles.tilesetColumns, (const char *) tilesetColumnsBytes, tilesetColumnsLength);
		g_string_append_len(component->tilemapTiles.tilesetRows, (const char *) tilesetRowsBytes, tilesetRowsLength);
		g_string_append_len(component->tilemapTiles.tilesetIds, (const char *) tilesetIdsBytes, tilesetIdsLength);
	}
}

static void onAuthorityChange(ClientContext *context, const Worker_AuthorityChangeOp *op)
{
	Entity *entity = g_hash_table_lookup(context->entities, &op->entity_id);
	if(entity == NULL) {
		shovelerLogWarning(
			"Received authority change for entity %"PRId64" component %"PRIu32" but entity is not in view, ignoring.",
			op->entity_id,
			op->component_id);
		return;
	}

	Component *component = g_hash_table_lookup(entity->components, &op->component_id);
	if(component == NULL) {
		shovelerLogWarning(
			"Received authority change for entity %"PRId64" component %"PRIu32" but component is not in view, ignoring.",
			op->entity_id,
			op->component_id);
		return;
	}

	bool newAuthority = op->authority == WORKER_AUTHORITY_AUTHORITATIVE;
	if(!component->authoritative != newAuthority) {
		return;
	}

	component->authoritative = newAuthority;

	if(op->authority == WORKER_AUTHORITY_AUTHORITATIVE) {
		if(op->component_id == shovelerWorkerSchemaComponentIdClient) {
			shovelerLogTrace("Gained client authority over entity %lld.", op->entity_id);
			context->clientEntityId = op->entity_id;
			context->clientPingTickCallback = shovelerExecutorSchedulePeriodic(context->executor, 0, clientPingTimeoutMs, clientPingTick, context);
		}
	} else if(op->authority == WORKER_AUTHORITY_NOT_AUTHORITATIVE) {
		if(op->component_id == shovelerWorkerSchemaComponentIdClient) {
			shovelerLogWarning("Lost client authority over entity %lld.", op->entity_id);
			context->clientEntityId = 0;
			shovelerExecutorRemoveCallback(context->executor, context->clientPingTickCallback);
		}
	}
}

static void clientPingTick(void *clientContextPointer)
{
	ClientContext *context = (ClientContext *) clientContextPointer;

	Schema_ComponentUpdate *componentUpdate = Schema_CreateComponentUpdate();
	Schema_Object *fields = Schema_GetComponentUpdateFields(componentUpdate);
	Schema_AddInt64(fields, shovelerWorkerSchemaClientHeartbeatPingFieldIdLastUpdatedTime, g_get_monotonic_time());

	Worker_ComponentUpdate update;
	update.component_id = shovelerWorkerSchemaComponentIdClientHeartbeatPing;
	update.schema_type = componentUpdate;

	Worker_UpdateParameters updateParameters;
	updateParameters.loopback = WORKER_COMPONENT_UPDATE_LOOPBACK_NONE;

	Worker_Connection_SendComponentUpdate(context->connection, context->clientEntityId, &update, &updateParameters);
	shovelerLogTrace("Sent client heartbeat ping update.");
}

static void clientDirectionChange(void *clientContextPointer) {
	ClientContext *context = (ClientContext *) clientContextPointer;

	if(rand() % 100 >= directionChangeChancePercent) {
		return;
	}

	context->direction = (context->direction + 1 + (rand() % 3)) % 4;
	shovelerLogTrace("Changing direction to %u.", context->direction);
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

static void move(ClientContext *context, Component *positionComponent, int dtMs) {
	ShovelerVector3 coordinates = positionComponent->position;

	float s = 0.001f * dtMs * velocity;

	switch(context->direction) {
		case kUp:
			coordinates.values[1] += s;
			break;
		case kDown:
			coordinates.values[1] -= s;
			break;
		case kLeft:
			coordinates.values[0] -= s;
			break;
		case kRight:
			coordinates.values[0] += s;
			break;
	}

	if(!validatePosition(context, coordinates)) {
		clientDirectionChange(context);
		return;
	}

	{
		Schema_ComponentUpdate *componentUpdate = Schema_CreateComponentUpdate();
		Schema_Object *fields = Schema_GetComponentUpdateFields(componentUpdate);
		Schema_Object *coordinatesObject = Schema_AddObject(fields, shovelerWorkerSchemaPositionFieldIdCoordinates);
		Schema_AddFloat(coordinatesObject, shovelerWorkerSchemaVector3FieldIdX, coordinates.values[0]);
		Schema_AddFloat(coordinatesObject, shovelerWorkerSchemaVector3FieldIdY, coordinates.values[1]);
		Schema_AddFloat(coordinatesObject, shovelerWorkerSchemaVector3FieldIdZ, coordinates.values[2]);

		Worker_ComponentUpdate update;
		update.component_id = shovelerWorkerSchemaComponentIdPosition;
		update.schema_type = componentUpdate;

		Worker_UpdateParameters updateParameters;
		updateParameters.loopback = WORKER_COMPONENT_UPDATE_LOOPBACK_NONE;

		Worker_Connection_SendComponentUpdate(context->connection, context->clientEntityId, &update, &updateParameters);
		shovelerLogTrace("Sent position update for client entity %lld to (%.2f, %.2f, %.2f).", context->clientEntityId, coordinates.values[0], coordinates.values[1], coordinates.values[2]);
	}

	ShovelerVector3 improbablePosition = shovelerVector3(coordinates.values[0], coordinates.values[2], coordinates.values[1]);
	ShovelerVector3 diff = shovelerVector3LinearCombination(1.0f, improbablePosition, -1.0f, context->lastImprobablePosition);
	float difference2 = shovelerVector3Dot(diff, diff);
	if(difference2 > improbablePositionUpdateDistance) {
		Schema_ComponentUpdate *componentUpdate = Schema_CreateComponentUpdate();
		Schema_Object *fields = Schema_GetComponentUpdateFields(componentUpdate);
		Schema_Object *coordinatesObject = Schema_AddObject(fields, shovelerWorkerSchemaImprobablePositionFieldIdCoords);
		Schema_AddDouble(coordinatesObject, shovelerWorkerSchemaImprobableCoordinatesFieldIdX, improbablePosition.values[0]);
		Schema_AddDouble(coordinatesObject, shovelerWorkerSchemaImprobableCoordinatesFieldIdY, improbablePosition.values[1]);
		Schema_AddDouble(coordinatesObject, shovelerWorkerSchemaImprobableCoordinatesFieldIdZ, improbablePosition.values[2]);

		Worker_ComponentUpdate update;
		update.component_id = shovelerWorkerSchemaComponentIdImprobablePosition;
		update.schema_type = componentUpdate;

		Worker_UpdateParameters updateParameters;
		updateParameters.loopback = WORKER_COMPONENT_UPDATE_LOOPBACK_NONE;

		Worker_Connection_SendComponentUpdate(context->connection, context->clientEntityId, &update, &updateParameters);
		shovelerLogTrace("Sent Improbable position update for client entity %lld to (%.2f, %.2f, %.2f).", context->clientEntityId, improbablePosition.values[0], improbablePosition.values[1], improbablePosition.values[2]);

		context->lastImprobablePosition = improbablePosition;
	}
}

static bool validatePosition(ClientContext *context, ShovelerVector3 coordinates) {
	ShovelerVector3 topRight = coordinates;
	topRight.values[0] += 0.5 * characterSize;
	topRight.values[1] = 0.5 * characterSize;
	if(!validatePoint(context, topRight)) {
		return false;
	}

	ShovelerVector3 topLeft = coordinates;
	topLeft.values[0] -= 0.5 * characterSize;
	topLeft.values[1] += 0.5 * characterSize;
	if(!validatePoint(context, topLeft)) {
		return false;
	}

	ShovelerVector3 bottomRight = coordinates;
	bottomRight.values[0] += 0.5 * characterSize;
	bottomRight.values[1] -= 0.5 * characterSize;
	if(!validatePoint(context, bottomRight)) {
		return false;
	}

	ShovelerVector3 bottomLeft = coordinates;
	bottomLeft.values[0] -= 0.5 * characterSize;
	bottomLeft.values[1] -= 0.5 * characterSize;
	if(!validatePoint(context, bottomLeft)) {
		return false;
	}

	return true;
}

static bool validatePoint(ClientContext *context, ShovelerVector3 coordinates) {
	const int numChunkColumns = 2 * halfMapWidth / chunkSize;
	const int numChunkRows = 2 * halfMapHeight / chunkSize;

	double x = coordinates.values[0];
	double z = coordinates.values[1];
	int chunkX, chunkZ, tileX, tileZ;
	worldToTile(x, z, &chunkX, &chunkZ, &tileX, &tileZ);

	if(chunkX < 0 || chunkX >= numChunkColumns || chunkZ < 0 || chunkZ >= numChunkRows || tileX < 0 || tileX >= chunkSize || tileZ < 0 || tileZ >= chunkSize) {
		shovelerLogTrace("Position (%.2f, %.2f, %.2f) validates to false because tile coordinates are invalid.", coordinates.values[0], coordinates.values[1], coordinates.values[2]);
		return false;
	}

	int64_t chunkBackgroundEntityId = getChunkBackgroundEntityId(chunkX, chunkZ);
	TilemapTiles *tiles = getChunkBackgroundTiles(context, chunkBackgroundEntityId);
	if(!tiles) {
		shovelerLogTrace("Position (%.2f, %.2f, %.2f) validates to false because background tiles are empty.", coordinates.values[0], coordinates.values[1], coordinates.values[2]);
		return false;
	}

	char tilesetColumn = tiles->tilesetColumns->str[tileZ * chunkSize + tileX];
	if(tilesetColumn > 2) { // tile isn't grass
		shovelerLogTrace("Position (%.2f, %.2f, %.2f) validates to false because tile isn't grass.", coordinates.values[0], coordinates.values[1], coordinates.values[2]);
		return false;
	}

	return true;
}

static int64_t getChunkBackgroundEntityId(int chunkX, int chunkZ)
{
	const int numChunkColumns = 2 * halfMapWidth / chunkSize;
	const int numChunkRows = 2 * halfMapHeight / chunkSize;

	if(chunkX < 0 || chunkX >= numChunkColumns || chunkZ < 0 || chunkZ >= numChunkRows) {
		shovelerLogWarning("Cannot resolve chunk background entity id for out of range chunk at (%d, %d).", chunkX, chunkZ);
		return 0;
	}

	return firstChunkEntityId + 3 * chunkX * numChunkColumns + 3 * chunkZ;
}

static TilemapTiles *getChunkBackgroundTiles(ClientContext *context, int64_t chunkBackgroundEntityId)
{
	Entity *entity = g_hash_table_lookup(context->entities, &chunkBackgroundEntityId);
	if(entity == NULL) {
		return NULL;
	}

	int32_t componentId = shovelerWorkerSchemaComponentIdTilemapTiles;
	Component *component = g_hash_table_lookup(entity->components, &componentId);
	if(component == NULL) {
		return NULL;
	}

	return &component->tilemapTiles;
}

static ShovelerVector2 tileToWorld(int chunkX, int chunkZ, int tileX, int tileZ)
{
	return shovelerVector2(
		(float) (-halfMapWidth + chunkX * chunkSize + tileX),
		(float) (-halfMapHeight + chunkZ * chunkSize + tileZ));
}

static void worldToTile(double x, double z, int *outputChunkX, int *outputChunkZ, int *outputTileX, int *outputTileZ)
{
	double diffX = x + halfMapWidth;
	double diffZ = z + halfMapHeight;

	*outputChunkX = (int) floor(diffX / chunkSize);
	*outputChunkZ = (int) floor(diffZ / chunkSize);

	*outputTileX = (int) floor(diffX - *outputChunkX * chunkSize);
	*outputTileZ = (int) floor(diffZ - *outputChunkZ * chunkSize);
}

static void freeEntity(void *entityPointer)
{
	Entity *entity = entityPointer;
	g_hash_table_destroy(entity->components);
	free(entity);
}

static void freeComponent(void *componentPointer)
{
	Component *component = componentPointer;

	if(component->componentId == shovelerWorkerSchemaComponentIdTilemapTiles) {
		g_string_free(component->tilemapTiles.tilesetRows, /* free_segment */ true);
		g_string_free(component->tilemapTiles.tilesetColumns, /* free_segment */ true);
		g_string_free(component->tilemapTiles.tilesetIds, /* free_segment */ true);
	}

	free(component);
}
