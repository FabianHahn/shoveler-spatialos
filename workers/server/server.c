#include <assert.h> // assert
#include <errno.h> // errno
#include <inttypes.h> // PRIu32 PRId64
#include <stdlib.h> // srand malloc free
#include <string.h> // strerror
#include <time.h> // time

#include <glib.h>
#include <improbable/c_schema.h>
#include <improbable/c_worker.h>
#include <shoveler/color.h>
#include <shoveler/component/position.h>
#include <shoveler/connect.h>
#include <shoveler/executor.h>
#include <shoveler/log.h>
#include <shoveler/schema.h>
#include <shoveler/types.h>
#include <shoveler/worker_log.h>

#include "configuration.h"

const int tickRateHz = 100;
const int64_t maxHeartbeatTimeoutMs = 5000;
const int clientCleanupTickRateHz = 2;
const int halfMapWidth = 100;
const int halfMapHeight = 100;
const int chunkSize = 10;
const int64_t firstChunkEntityId = 12;
const int numPlayerPositionAttempts = 10;
const int64_t cubeDrawableEntityId = 2;
const int64_t pointDrawableEntityId = 4;
const int64_t characterAnimationTilesetEntityId = 5;
const int64_t character2AnimationTilesetEntityId = 6;
const int64_t character3AnimationTilesetEntityId = 7;
const int64_t character4AnimationTilesetEntityId = 8;
const int64_t canvasEntityId = 9;

typedef struct {
	GString *tilesetRows;
	GString *tilesetColumns;
	GString *tilesetIds;
} TilemapTiles;

typedef struct {
	float colorHue;
	float colorSaturation;
} ClientInfo;

typedef struct {
	uint32_t componentId;
	bool authoritative;
	union {
		TilemapTiles tilemapTiles;
		ClientInfo clientInfo;
	};
} Component;

typedef struct {
	int64_t entityId;
	GHashTable *components;
} Entity;

typedef struct {
	int64_t entityId;
	char *workerId;
	int64_t lastPong;
} Client;

typedef struct {
	Worker_Connection *connection;
	ShovelerServerConfiguration configuration;
	GHashTable *entities;
	GHashTable *clients;
	int numAuthoritativeComponents;
	int numEntitiesLastTick;
	int numAuthoritativeComponentsLastTick;
	int characterCounter;
	bool disconnected;
} ServerContext;

static void clientCleanupTick(void *contextPointer);
static void updateTickMetrics(ServerContext *context);
static void onAddComponent(ServerContext *context, const Worker_AddComponentOp *op);
static void onComponentUpdate(ServerContext *context, const Worker_ComponentUpdateOp *op);
static void onAuthorityChange(ServerContext *context, const Worker_AuthorityChangeOp *op);
static void onCommandRequest(ServerContext *context, const Worker_CommandRequestOp *op);
static void onCreateClientEntityRequest(ServerContext *context, const Worker_CommandRequestOp *op);
static void onClientSpawnCubeRequest(ServerContext *context, const Worker_CommandRequestOp *op);
static void onDigHoleRequest(ServerContext *context, const Worker_CommandRequestOp *op);
static void onUpdateResourceRequest(ServerContext *context, const Worker_CommandRequestOp *op);
static Client *getOrCreateClient(ServerContext *context, int64_t entityId);
static ShovelerVector3 getNewPlayerPosition(ServerContext *context, Schema_Object *requestObject);
static int64_t getChunkBackgroundEntityId(int chunkX, int chunkZ);
static TilemapTiles *getChunkBackgroundTiles(ServerContext *context, int64_t chunkBackgroundEntityId);
static ShovelerVector2 tileToWorld(int chunkX, int chunkZ, int tileX, int tileZ);
static void worldToTile(double x, double z, int *outputChunkX, int *outputChunkZ, int *outputTileX, int *outputTileZ);
static const char *getSpecificWorkerAttribute(const Worker_WorkerAttributes *workerAttributes);
static ShovelerVector3 remapImprobablePosition(const ShovelerVector3 *coordinates, bool isTiles);
static ShovelerVector3 remapPosition(const ShovelerVector3 *coordinates, bool isTiles);
static ShovelerVector4 colorFromHsv(float h, float s, float v);
static void freeEntity(void *entityPointer);
static void freeComponent(void *componentPointer);
static void freeClient(void *clientPointer);

int main(int argc, char **argv)
{
	srand(time(NULL));

	if(argc != 5) {
		fprintf(stderr, "Usage:\n\t%s LOG_FILE_LOCATION WORKER_ID HOSTNAME PORT", argv[0]);
		return 1;
	}

	const char *logFileLocation = argv[1];

	FILE *logFile = fopen(logFileLocation, "w+");
	if(logFile == NULL) {
		fprintf(stderr, "Failed to open output log file at %s: %s", logFileLocation, strerror(errno));
		return 1;
	}

	shovelerLogInit("shoveler-spatialos/", SHOVELER_LOG_LEVEL_INFO_UP, logFile);

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
	connectionParameters.worker_type = "ShovelerServer";
	connectionParameters.network.connection_type = WORKER_NETWORK_CONNECTION_TYPE_MODULAR_TCP;
	connectionParameters.network.modular_tcp.security_type = WORKER_NETWORK_SECURITY_TYPE_INSECURE;
	connectionParameters.default_component_vtable = &componentVtable;
	connectionParameters.logsink_count = 1;
	connectionParameters.logsinks = &logsink;
	connectionParameters.enable_logging_at_startup = true;

	Worker_Connection *connection = shovelerWorkerConnect(argc, argv, /* argumentOffset */ 1, &connectionParameters);
	assert(connection != NULL);
	if(!Worker_Connection_IsConnected(connection)) {
		shovelerLogError("Failed to connect to SpatialOS deployment: %s", Worker_Connection_GetConnectionStatusDetailString(connection));
		Worker_Connection_Destroy(connection);
		return EXIT_FAILURE;
	}
	shovelerLogInfo("Connected to SpatialOS deployment!");

	ServerContext context;
	context.connection = connection;
	shovelerServerGetWorkerConfiguration(connection, &context.configuration);
	context.entities = g_hash_table_new_full(g_int64_hash, g_int64_equal, /* key_destroy_func */ NULL, freeEntity);
	context.clients = g_hash_table_new_full(g_int64_hash, g_int64_equal, /* key_destroy_func */ NULL, freeClient);
	context.numAuthoritativeComponents = 0;
	context.numEntitiesLastTick = 0;
	context.numAuthoritativeComponentsLastTick = 0;
	context.characterCounter = 0;
	context.disconnected = false;

	ShovelerExecutor *tickExecutor = shovelerExecutorCreateDirect();
	int clientCleanupTickPeriod = (int) (1000.0 / (double) clientCleanupTickRateHz);
	shovelerExecutorSchedulePeriodic(tickExecutor, 0, clientCleanupTickPeriod, clientCleanupTick, &context);

	const uint32_t tickTimeoutMillis = 1000 / tickRateHz;
	while(!context.disconnected) {
		Worker_OpList *opList = Worker_Connection_GetOpList(connection, tickTimeoutMillis);
		for(size_t i = 0; i < opList->op_count; ++i) {
			Worker_Op *op = &opList->ops[i];
			switch(op->op_type) {
				case WORKER_OP_TYPE_DISCONNECT:
					shovelerLogInfo("Disconnected from SpatialOS with code %d: %s", op->op.disconnect.connection_status_code, op->op.disconnect.reason);
					context.disconnected = true;
					break;
				case WORKER_OP_TYPE_FLAG_UPDATE:
					// ignore
					break;
				case WORKER_OP_TYPE_LOG_MESSAGE:
					// deprecated and can be ignored, we receive all log messages through logsink already.
					break;
				case WORKER_OP_TYPE_METRICS:
					Worker_Connection_SendMetrics(connection, &op->op.metrics.metrics);
					break;
				case WORKER_OP_TYPE_CRITICAL_SECTION:
					// ignore
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
					shovelerLogInfo(
						"Received create entity %"PRId64" response for request %"PRId64" with status code %d: %s",
						op->op.create_entity_response.entity_id,
						op->op.create_entity_response.request_id,
						op->op.create_entity_response.status_code,
						op->op.create_entity_response.message);
					break;
				case WORKER_OP_TYPE_DELETE_ENTITY_RESPONSE:
					shovelerLogInfo(
						"Received delete entity %"PRId64" response for request %"PRId64" with status code %d: %s",
						op->op.delete_entity_response.entity_id,
						op->op.delete_entity_response.request_id,
						op->op.delete_entity_response.status_code,
						op->op.delete_entity_response.message);
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
					onCommandRequest(&context, &op->op.command_request);
					break;
				case WORKER_OP_TYPE_COMMAND_RESPONSE:
					shovelerLogTrace("WORKER_OP_TYPE_COMMAND_RESPONSE");
					break;
			}
		}
		Worker_OpList_Destroy(opList);

		shovelerExecutorUpdateNow(tickExecutor);

		updateTickMetrics(&context);
	}
	shovelerLogInfo("Exiting main loop, goodbye.");

	Worker_Connection_Destroy(connection);
	shovelerExecutorFree(tickExecutor);
	g_hash_table_destroy(context.entities);
	g_hash_table_destroy(context.clients);
	shovelerLogTerminate();

	return EXIT_SUCCESS;
}

static void clientCleanupTick(void *contextPointer)
{
	ServerContext *context = contextPointer;

	int64_t now = g_get_monotonic_time();

	GHashTableIter iter;
	uint64_t *entityId;
	Client *client;
	g_hash_table_iter_init(&iter, context->clients);
	while(g_hash_table_iter_next(&iter, (gpointer *) &entityId, (gpointer *) &client)) {
		int64_t diff = now - client->lastPong;
		if(diff > 1000 * maxHeartbeatTimeoutMs) {
			Worker_RequestId requestId = Worker_Connection_SendDeleteEntityRequest(context->connection, client->entityId, NULL);

			shovelerLogWarning(
				"Sent remove client entity %lld request %lld of worker %s because it exceeded the maximum heartbeat timeout of %lldms: Last pong = %lld, now = %lld.",
				client->entityId,
				requestId,
				client->workerId,
				maxHeartbeatTimeoutMs,
				client->lastPong,
				now);
		}
	}
}

static void updateTickMetrics(ServerContext *context)
{
	bool viewUpdated = false;

	if(g_hash_table_size(context->entities) != context->numEntitiesLastTick) {
		context->numEntitiesLastTick = g_hash_table_size(context->entities);
		viewUpdated = true;
	}

	if(context->numAuthoritativeComponents != context->numAuthoritativeComponentsLastTick) {
		context->numAuthoritativeComponentsLastTick = context->numAuthoritativeComponents;
		viewUpdated = true;
	}

	if(viewUpdated) {
		shovelerLogInfo(
			"View updated: %d entities, %d authoritative components.",
			context->numEntitiesLastTick,
			context->numAuthoritativeComponentsLastTick);
	}
}

static void onAddComponent(ServerContext *context, const Worker_AddComponentOp *op)
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

	if(component->componentId == shovelerWorkerSchemaComponentIdTilemapTiles) {
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
	} else if (component->componentId == shovelerWorkerSchemaComponentIdClientInfo) {
		component->clientInfo.colorHue = Schema_GetFloat(fields, shovelerWorkerSchemaClientInfoFieldIdColorHue);
		component->clientInfo.colorSaturation = Schema_GetFloat(fields, shovelerWorkerSchemaClientInfoFieldIdColorSaturation);
	}

	g_hash_table_insert(entity->components, &component->componentId, component);
}

static void onComponentUpdate(ServerContext *context, const Worker_ComponentUpdateOp *op)
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

	if(op->update.component_id == shovelerWorkerSchemaComponentIdClientHeartbeatPing) {
		if(Schema_GetInt64Count(fields, shovelerWorkerSchemaClientHeartbeatPingFieldIdLastUpdatedTime) == 0) {
			return;
		}
		int64_t lastUpdatedTime = Schema_GetInt64(fields, shovelerWorkerSchemaClientHeartbeatPingFieldIdLastUpdatedTime);

		Worker_ComponentUpdate pongComponentUpdate;
		pongComponentUpdate.component_id = shovelerWorkerSchemaComponentIdClientHeartbeatPong;
		pongComponentUpdate.schema_type = Schema_CreateComponentUpdate();
		Schema_Object *pongFields = Schema_GetComponentUpdateFields(pongComponentUpdate.schema_type);
		Schema_AddInt64(pongFields, shovelerWorkerSchemaClientHeartbeatPongFieldIdLastUpdatedTime, lastUpdatedTime);

		Worker_Connection_SendComponentUpdate(context->connection, op->entity_id, &pongComponentUpdate, /* update_parameters */ NULL);

		Client *client = getOrCreateClient(context, op->entity_id);
		client->lastPong = g_get_monotonic_time();
		shovelerLogTrace("Reflected client %"PRId64" heartbeat pong update.", op->entity_id);
	}
}

static void onAuthorityChange(ServerContext *context, const Worker_AuthorityChangeOp *op)
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

	if(component->authoritative) {
		context->numAuthoritativeComponents++;
	} else {
		context->numAuthoritativeComponents--;
	}

	if(component->componentId == shovelerWorkerSchemaComponentIdClientHeartbeatPong) {
		if(component->authoritative) {
			Client *client = getOrCreateClient(context, op->entity_id);
			client->lastPong = g_get_monotonic_time() + 1000 * maxHeartbeatTimeoutMs;
			shovelerLogInfo("Added authoritative client %lld, last pong initialized with grace period to %lld.", op->entity_id, client->lastPong);
		} else {
			g_hash_table_remove(context->clients, &op->entity_id);
			shovelerLogInfo("Removed authoritative client %lld.", op->entity_id);
		}
	}
}

static void onCommandRequest(ServerContext *context, const Worker_CommandRequestOp *op)
{
	if(op->request.component_id != shovelerWorkerSchemaComponentIdBootstrap) {
		shovelerLogWarning(
			"Received command request for entity %"PRId64" component %"PRIu32" which is not the bootstrap component, ignoring.",
			op->entity_id,
			op->request.component_id);
		return;
	}

	switch(op->request.command_index) {
		case shovelerWorkerSchemaBootstrapCommandIdCreateClientEntity:
			onCreateClientEntityRequest(context, op);
			break;
		case shovelerWorkerSchemaBootstrapCommandIdClientSpawnCube:
			onClientSpawnCubeRequest(context, op);
			break;
		case shovelerWorkerSchemaBootstrapCommandIdDigHole:
			onDigHoleRequest(context, op);
			break;
		case shovelerWorkerSchemaBootstrapCommandIdUpdateResource:
			onUpdateResourceRequest(context, op);
			break;
	}
}

static void onCreateClientEntityRequest(ServerContext *context, const Worker_CommandRequestOp *op)
{
	shovelerLogInfo("Received create client entity request from '%s'.", op->caller_worker_id);

	Schema_Object *requestObject = Schema_GetCommandRequestObject(op->request.schema_type);

	ShovelerVector3 playerImprobablePosition = getNewPlayerPosition(context, requestObject);
	ShovelerVector3 playerPosition = remapImprobablePosition(&playerImprobablePosition, context->configuration.gameType == SHOVELER_WORKER_GAME_TYPE_TILES);

	Worker_ComponentData clientEntityComponentData[13] = {0};

	clientEntityComponentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("client");

	// shoveler.Client
	clientEntityComponentData[1].component_id = shovelerWorkerSchemaComponentIdClient;
	clientEntityComponentData[1].schema_type = Schema_CreateComponentData();
	Schema_Object *client = Schema_GetComponentDataFields(clientEntityComponentData[1].schema_type);
	Schema_AddEntityId(client, shovelerWorkerSchemaClientFieldIdPosition, 0);

	// shoveler.ClientHeartbeatPing
	clientEntityComponentData[2].component_id = shovelerWorkerSchemaComponentIdClientHeartbeatPing;
	clientEntityComponentData[2].schema_type = Schema_CreateComponentData();
	Schema_Object *clientHeartbeatPing = Schema_GetComponentDataFields(clientEntityComponentData[2].schema_type);
	Schema_AddInt64(clientHeartbeatPing, shovelerWorkerSchemaClientHeartbeatPingFieldIdLastUpdatedTime, 0);

	// shoveler.ClientHeartbeatPong
	clientEntityComponentData[3].component_id = shovelerWorkerSchemaComponentIdClientHeartbeatPong;
	clientEntityComponentData[3].schema_type = Schema_CreateComponentData();
	Schema_Object *clientHeartbeatPong = Schema_GetComponentDataFields(clientEntityComponentData[3].schema_type);
	Schema_AddInt64(clientHeartbeatPong, shovelerWorkerSchemaClientHeartbeatPongFieldIdLastUpdatedTime, 0);

	clientEntityComponentData[4] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
	clientEntityComponentData[5] = shovelerWorkerSchemaCreateImprobablePositionComponent(
		playerImprobablePosition.values[0], playerImprobablePosition.values[1], playerImprobablePosition.values[2]);
	clientEntityComponentData[6] = shovelerWorkerSchemaCreatePositionComponent(playerPosition);

	clientEntityComponentData[7] = shovelerWorkerSchemaCreateImprobableInterestComponent();
	Schema_Object *clientComponentInterest = shovelerWorkerSchemaAddImprobableInterestForComponent(
		&clientEntityComponentData[7], shovelerWorkerSchemaComponentIdClient);
	Schema_Object *relativeQuery = shovelerWorkerSchemaAddImprobableInterestComponentQuery(clientComponentInterest);
	shovelerWorkerSchemaSetImprobableInterestQueryRelativeBoxConstraint(relativeQuery, 20.5, 9999.0, 20.5);
	shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(relativeQuery, shovelerWorkerSchemaComponentIdLight);
	shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(relativeQuery, shovelerWorkerSchemaComponentIdModel);
	shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(relativeQuery, shovelerWorkerSchemaComponentIdPosition);
	shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(relativeQuery, shovelerWorkerSchemaComponentIdSprite);
	shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(relativeQuery, shovelerWorkerSchemaComponentIdTilemapTiles);
	Schema_Object *heartbeatQuery = shovelerWorkerSchemaAddImprobableInterestComponentQuery(clientComponentInterest);
	shovelerWorkerSchemaSetImprobableInterestQueryRelativeSphereConstraint(heartbeatQuery, 0.0);
	shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(heartbeatQuery, shovelerWorkerSchemaComponentIdClientHeartbeatPong);

	// improbable.EntityAcl
	clientEntityComponentData[8] = shovelerWorkerSchemaCreateImprobableEntityAclComponent();
	shovelerWorkerSchemaAddImprobableEntityAclReadStatic(&clientEntityComponentData[8], "client");
	shovelerWorkerSchemaAddImprobableEntityAclReadStatic(&clientEntityComponentData[8], "server");
	Schema_Object *entityAcl = Schema_GetComponentDataFields(clientEntityComponentData[8].schema_type);
	const char *specificClientAttribute = getSpecificWorkerAttribute(&op->caller_attribute_set);
	uint32_t specificClientAttributeLength = strlen(specificClientAttribute);
	uint8_t *specificClientAttributeBuffer = Schema_AllocateBuffer(entityAcl, specificClientAttributeLength);
	memcpy(specificClientAttributeBuffer, specificClientAttribute, specificClientAttributeLength);
	uint32_t clientAuthoritativeComponentIds[] = {
		shovelerWorkerSchemaComponentIdClient,
		shovelerWorkerSchemaComponentIdClientHeartbeatPing,
		shovelerWorkerSchemaComponentIdImprobableInterest,
		shovelerWorkerSchemaComponentIdImprobablePosition,
		shovelerWorkerSchemaComponentIdPosition,
	};
	for(uint32_t i = 0; i < 5; i++) {
		Schema_Object *componentWriteAclEntry = Schema_AddObject(entityAcl, shovelerWorkerSchemaImprobableEntityAclFieldIdComponentWriteAcl);
		Schema_AddUint32(componentWriteAclEntry, SCHEMA_MAP_KEY_FIELD_ID, clientAuthoritativeComponentIds[i]);
		Schema_Object *componentWriteAclClientWorkerRequirementSet = Schema_AddObject(componentWriteAclEntry, SCHEMA_MAP_VALUE_FIELD_ID);
		Schema_Object *componentWriteAclClientAttributeSet = Schema_AddObject(componentWriteAclClientWorkerRequirementSet, shovelerWorkerSchemaImprobableWorkerRequirementSetFieldIdAttributeSet);
		Schema_AddBytes(componentWriteAclClientAttributeSet, shovelerWorkerSchemaImprobableWorkerAttributeSetFieldIdAttribute, specificClientAttributeBuffer, specificClientAttributeLength);
	}
	Schema_Object *componentWriteAclClientHeartbeatPongEntry = Schema_AddObject(entityAcl, shovelerWorkerSchemaImprobableEntityAclFieldIdComponentWriteAcl);
	Schema_AddUint32(componentWriteAclClientHeartbeatPongEntry, SCHEMA_MAP_KEY_FIELD_ID, shovelerWorkerSchemaComponentIdClientHeartbeatPong);
	Schema_Object *componentWriteAclClientHeartbeatPongWorkerRequirementSet = Schema_AddObject(componentWriteAclClientHeartbeatPongEntry, SCHEMA_MAP_VALUE_FIELD_ID);
	Schema_Object *componentWriteAclClientHeartbeatPongAttributeSet = Schema_AddObject(componentWriteAclClientHeartbeatPongWorkerRequirementSet, shovelerWorkerSchemaImprobableWorkerRequirementSetFieldIdAttributeSet);
	Schema_AddBytes(componentWriteAclClientHeartbeatPongAttributeSet, shovelerWorkerSchemaImprobableWorkerAttributeSetFieldIdAttribute, (const uint8_t *) "server", strlen("server"));

	float hue = 0.0f;
	float saturation = 0.0f;
	if(context->configuration.gameType == SHOVELER_WORKER_GAME_TYPE_TILES) {
		int modulo = context->characterCounter++ % 4;
		uint64_t tilesetEntityId = characterAnimationTilesetEntityId;
		if(modulo == 1) {
			tilesetEntityId = character2AnimationTilesetEntityId;
		} else if(modulo == 2) {
			tilesetEntityId = character3AnimationTilesetEntityId;
		} else if(modulo == 3) {
			tilesetEntityId = character4AnimationTilesetEntityId;
		}

		// shoveler.Sprite
		clientEntityComponentData[9].component_id = shovelerWorkerSchemaComponentIdSprite;
		clientEntityComponentData[9].schema_type = Schema_CreateComponentData();
		Schema_Object *sprite = Schema_GetComponentDataFields(clientEntityComponentData[9].schema_type);
		Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdPosition, 0);
		Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingX, SHOVELER_COORDINATE_MAPPING_POSITIVE_X);
		Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingY, SHOVELER_COORDINATE_MAPPING_POSITIVE_Y);
		Schema_AddBool(sprite, shovelerWorkerSchemaSpriteFieldIdEnableCollider, false);
		Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdCanvas, canvasEntityId);
		Schema_AddInt32(sprite, shovelerWorkerSchemaSpriteFieldIdLayer, 1);
		Schema_Object *size = Schema_AddObject(sprite, shovelerWorkerSchemaSpriteFieldIdSize);
		Schema_AddFloat(size, shovelerWorkerSchemaVector2FieldIdX, 1.0f);
		Schema_AddFloat(size, shovelerWorkerSchemaVector2FieldIdY, 1.0f);
		Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdTileSprite, 0);

		// shoveler.TileSprite
		clientEntityComponentData[10].component_id = shovelerWorkerSchemaComponentIdTileSprite;
		clientEntityComponentData[10].schema_type = Schema_CreateComponentData();
		Schema_Object *tileSprite = Schema_GetComponentDataFields(clientEntityComponentData[10].schema_type);
		Schema_AddEntityId(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdMaterial, canvasEntityId);
		Schema_AddEntityId(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdTileset, tilesetEntityId);
		Schema_AddInt32(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdTilesetColumn, 0);
		Schema_AddInt32(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdTilesetRow, 0);

		// shoveler.TileSpriteAnimation
		clientEntityComponentData[11].component_id = shovelerWorkerSchemaComponentIdTileSpriteAnimation;
		clientEntityComponentData[11].schema_type = Schema_CreateComponentData();
		Schema_Object *tileSpriteAnimation = Schema_GetComponentDataFields(clientEntityComponentData[11].schema_type);
		Schema_AddEntityId(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdPosition, 0);
		Schema_AddEntityId(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdTileSprite, 0);
		Schema_AddEnum(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdPositionMappingX, SHOVELER_COORDINATE_MAPPING_POSITIVE_X);
		Schema_AddEnum(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdPositionMappingY, SHOVELER_COORDINATE_MAPPING_POSITIVE_Y);
		Schema_AddFloat(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdMoveAmountThreshold, 0.5f);
	} else {
		hue = (float) rand() / RAND_MAX;
		saturation = 0.5f + 0.5f * ((float) rand() / RAND_MAX);
		ShovelerVector4 playerParticleColor = colorFromHsv(hue, saturation, 0.9f);
		ShovelerVector4 playerLightColor = colorFromHsv(hue, saturation, 0.1f);

		clientEntityComponentData[9] = shovelerWorkerSchemaCreateMaterialParticleComponent(playerParticleColor);
		clientEntityComponentData[10] = shovelerWorkerSchemaCreateModelComponent(
			/* position */ 0,
			pointDrawableEntityId,
			/* material */ 0,
			/* rotation */ shovelerVector3(0.0f, 0.0f, 0.0f),
			/* scale */ shovelerVector3(0.1f, 0.1f, 0.1f),
			/* visible */ true,
			/* emitter */ true,
			/* castsShadow */ false,
			shovelerWorkerSchemaPolygonModeFill);
		clientEntityComponentData[11] = shovelerWorkerSchemaCreateLightComponent(
			/* position */ 0,
			shovelerWorkerSchemaLightTypePoint,
			/* width */ 1024,
			/* height */ 1024,
			/* samples */ 1,
			/* ambientFactor */ 0.01f,
			/* exponentialFactor */ 80.0f,
			/* color */ shovelerVector3(playerLightColor.values[0], playerLightColor.values[1], playerLightColor.values[2]));
	}

	// shoveler.ClientInfo
	clientEntityComponentData[12].component_id = shovelerWorkerSchemaComponentIdClientInfo;
	clientEntityComponentData[12].schema_type = Schema_CreateComponentData();
	Schema_Object *clientInfo = Schema_GetComponentDataFields(clientEntityComponentData[12].schema_type);
	uint32_t callerWorkerIdLength = strlen(op->caller_worker_id);
	uint8_t *callerWorkerIdBuffer = Schema_AllocateBuffer(clientInfo, callerWorkerIdLength);
	memcpy(callerWorkerIdBuffer, op->caller_worker_id, callerWorkerIdLength);
	Schema_AddBytes(clientInfo, shovelerWorkerSchemaClientInfoFieldIdWorkerId, callerWorkerIdBuffer, callerWorkerIdLength);
	Schema_AddFloat(clientInfo, shovelerWorkerSchemaClientInfoFieldIdColorHue, hue);
	Schema_AddFloat(clientInfo, shovelerWorkerSchemaClientInfoFieldIdColorSaturation, saturation);

	Worker_RequestId createEntityRequestId = Worker_Connection_SendCreateEntityRequest(
		context->connection,
		sizeof(clientEntityComponentData) / sizeof(clientEntityComponentData[0]),
		clientEntityComponentData,
		/* entity_id */ NULL,
		/* timeout_millis */ NULL);
	if(createEntityRequestId == -1) {
		shovelerLogError("Failed to send create entity request.");
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "entity creation failure");
		return;
	}

	shovelerLogInfo("Sent create entity request %"PRId64"", createEntityRequestId);

	Worker_CommandResponse commandResponse;
	commandResponse.component_id = op->request.component_id;
	commandResponse.command_index = op->request.command_index;
	commandResponse.schema_type = Schema_CreateCommandResponse();

	int8_t result = Worker_Connection_SendCommandResponse(context->connection, op->request_id, &commandResponse);
	if(result == WORKER_RESULT_FAILURE) {
		shovelerLogError("Failed to send create entity command response.");
	}
}

static void onClientSpawnCubeRequest(ServerContext *context, const Worker_CommandRequestOp *op)
{
	shovelerLogInfo("Received client spawn cube request from '%s'.", op->caller_worker_id);

	Schema_Object *requestObject = Schema_GetCommandRequestObject(op->request.schema_type);

	int64_t clientEntityId = Schema_GetEntityId(requestObject, shovelerWorkerSchemaClientSpawnCubeRequestFieldIdClient);
	Entity *clientEntity = g_hash_table_lookup(context->entities, &clientEntityId);
	if(clientEntity == NULL) {
		shovelerLogWarning("Received client spawn cube from %s for unknown client entity %"PRId64", ignoring.", op->caller_worker_id, clientEntityId);
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "unknown client entity");
		return;
	}

	uint32_t clientInfoComponentId = shovelerWorkerSchemaComponentIdClientInfo;
	Component *clientInfoComponent = g_hash_table_lookup(clientEntity->components, &clientInfoComponentId);
	if(clientInfoComponent == NULL) {
		shovelerLogWarning("Received client spawn cube from %s for client entity %"PRId64" without client info component, ignoring.", op->caller_worker_id, clientEntityId);
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "no client info component");
		return;
	}

	Schema_Object *positionObject = Schema_GetObject(requestObject, shovelerWorkerSchemaClientSpawnCubeRequestFieldIdPosition);
	Schema_Object *directionObject = Schema_GetObject(requestObject, shovelerWorkerSchemaClientSpawnCubeRequestFieldIdDirection);
	Schema_Object *rotationObject = Schema_GetObject(requestObject, shovelerWorkerSchemaClientSpawnCubeRequestFieldIdRotation);
	if(positionObject == NULL || directionObject == NULL || rotationObject == NULL) {
		shovelerLogWarning("Client spawn cube request doesn't contain position, direction and rotation - ignoring.");
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "no position, direction and rotation");
		return;
	}

	ShovelerVector3 requestPosition = shovelerVector3(
		Schema_GetFloat(positionObject, shovelerWorkerSchemaVector3FieldIdX),
		Schema_GetFloat(positionObject, shovelerWorkerSchemaVector3FieldIdY),
		Schema_GetFloat(positionObject, shovelerWorkerSchemaVector3FieldIdZ));
	ShovelerVector3 requestDirection = shovelerVector3(
		Schema_GetFloat(directionObject, shovelerWorkerSchemaVector3FieldIdX),
		Schema_GetFloat(directionObject, shovelerWorkerSchemaVector3FieldIdY),
		Schema_GetFloat(directionObject, shovelerWorkerSchemaVector3FieldIdZ));
	ShovelerVector3 requestRotation = shovelerVector3(
		Schema_GetFloat(rotationObject, shovelerWorkerSchemaVector3FieldIdX),
		Schema_GetFloat(rotationObject, shovelerWorkerSchemaVector3FieldIdY),
		Schema_GetFloat(rotationObject, shovelerWorkerSchemaVector3FieldIdZ));

	ShovelerVector3 normalizedDirection = shovelerVector3Normalize(requestDirection);
	ShovelerVector3 cubePosition = shovelerVector3LinearCombination(1.0f, requestPosition, 0.5f, normalizedDirection);

	ShovelerVector3 cubeImprobablePosition = remapPosition(&cubePosition, /* isTiles */ false);
	ShovelerVector4 cubeColor = colorFromHsv(clientInfoComponent->clientInfo.colorHue, clientInfoComponent->clientInfo.colorSaturation, 0.7f);


	Worker_ComponentData cubeEntityComponentData[7] = {0};

	// improbable.Metadata
	cubeEntityComponentData[0].component_id = shovelerWorkerSchemaComponentIdImprobableMetadata;
	cubeEntityComponentData[0].schema_type = Schema_CreateComponentData();
	Schema_Object *metadata = Schema_GetComponentDataFields(cubeEntityComponentData[0].schema_type);
	Schema_AddBytes(metadata, shovelerWorkerSchemaImprobableMetadataFieldIdEntityType, (const uint8_t *) "cube", strlen("cube"));

	// improbable.Persistence
	cubeEntityComponentData[1].component_id = shovelerWorkerSchemaComponentIdImprobablePersistence;
	cubeEntityComponentData[1].schema_type = Schema_CreateComponentData();

	// improbable.Position
	cubeEntityComponentData[2].component_id = shovelerWorkerSchemaComponentIdImprobablePosition;
	cubeEntityComponentData[2].schema_type = Schema_CreateComponentData();
	Schema_Object *improbablePosition = Schema_GetComponentDataFields(cubeEntityComponentData[2].schema_type);
	Schema_Object *coords = Schema_AddObject(improbablePosition, shovelerWorkerSchemaImprobablePositionFieldIdCoords);
	Schema_AddDouble(coords, shovelerWorkerSchemaImprobableCoordinatesFieldIdX, cubeImprobablePosition.values[0]);
	Schema_AddDouble(coords, shovelerWorkerSchemaImprobableCoordinatesFieldIdY, cubeImprobablePosition.values[1]);
	Schema_AddDouble(coords, shovelerWorkerSchemaImprobableCoordinatesFieldIdZ, cubeImprobablePosition.values[2]);

	// shoveler.Position
	cubeEntityComponentData[3].component_id = shovelerWorkerSchemaComponentIdPosition;
	cubeEntityComponentData[3].schema_type = Schema_CreateComponentData();
	Schema_Object *position = Schema_GetComponentDataFields(cubeEntityComponentData[3].schema_type);
	Schema_AddEnum(position, shovelerWorkerSchemaPositionFieldIdPositionType, SHOVELER_COMPONENT_POSITION_TYPE_ABSOLUTE);
	Schema_Object *coordinates = Schema_AddObject(position, shovelerWorkerSchemaPositionFieldIdCoordinates);
	Schema_AddFloat(coordinates, shovelerWorkerSchemaVector3FieldIdX, cubePosition.values[0]);
	Schema_AddFloat(coordinates, shovelerWorkerSchemaVector3FieldIdY, cubePosition.values[1]);
	Schema_AddFloat(coordinates, shovelerWorkerSchemaVector3FieldIdZ, cubePosition.values[2]);

	// improbable.EntityAcl
	cubeEntityComponentData[4].component_id = shovelerWorkerSchemaComponentIdImprobableEntityAcl;
	cubeEntityComponentData[4].schema_type = Schema_CreateComponentData();
	Schema_Object *entityAcl = Schema_GetComponentDataFields(cubeEntityComponentData[4].schema_type);
	Schema_Object *readAcl = Schema_AddObject(entityAcl, shovelerWorkerSchemaImprobableEntityAclFieldIdReadAcl);
	Schema_Object *readAclClientAttributeSet = Schema_AddObject(readAcl, shovelerWorkerSchemaImprobableWorkerRequirementSetFieldIdAttributeSet);
	Schema_AddBytes(readAclClientAttributeSet, shovelerWorkerSchemaImprobableWorkerAttributeSetFieldIdAttribute, (const uint8_t *) "client", strlen("client"));

	// shoveler.Material
	cubeEntityComponentData[5].component_id = shovelerWorkerSchemaComponentIdMaterial;
	cubeEntityComponentData[5].schema_type = Schema_CreateComponentData();
	Schema_Object *material = Schema_GetComponentDataFields(cubeEntityComponentData[5].schema_type);
	Schema_AddEnum(material, shovelerWorkerSchemaMaterialFieldIdType, shovelerWorkerSchemaMaterialTypeColor);
	Schema_Object *materialColor = Schema_AddObject(material, shovelerWorkerSchemaMaterialFieldIdColor);
	Schema_AddFloat(materialColor, shovelerWorkerSchemaVector4FieldIdX, cubeColor.values[0]);
	Schema_AddFloat(materialColor, shovelerWorkerSchemaVector4FieldIdY, cubeColor.values[1]);
	Schema_AddFloat(materialColor, shovelerWorkerSchemaVector4FieldIdZ, cubeColor.values[2]);
	Schema_AddFloat(materialColor, shovelerWorkerSchemaVector4FieldIdW, cubeColor.values[3]);

	// shoveler.Model
	cubeEntityComponentData[6].component_id = shovelerWorkerSchemaComponentIdModel;
	cubeEntityComponentData[6].schema_type = Schema_CreateComponentData();
	Schema_Object *model = Schema_GetComponentDataFields(cubeEntityComponentData[6].schema_type);
	Schema_AddEntityId(model, shovelerWorkerSchemaModelFieldIdPosition, 0);
	Schema_AddEntityId(model, shovelerWorkerSchemaModelFieldIdDrawable, cubeDrawableEntityId);
	Schema_AddEntityId(model, shovelerWorkerSchemaModelFieldIdMaterial, 0);
	Schema_Object *rotation = Schema_AddObject(model, shovelerWorkerSchemaModelFieldIdRotation);
	Schema_AddFloat(rotation, shovelerWorkerSchemaVector3FieldIdX, requestRotation.values[0]);
	Schema_AddFloat(rotation, shovelerWorkerSchemaVector3FieldIdY, requestRotation.values[1]);
	Schema_AddFloat(rotation, shovelerWorkerSchemaVector3FieldIdZ, requestRotation.values[2]);
	Schema_Object *scale = Schema_AddObject(model, shovelerWorkerSchemaModelFieldIdScale);
	Schema_AddFloat(scale, shovelerWorkerSchemaVector3FieldIdX, 0.25f);
	Schema_AddFloat(scale, shovelerWorkerSchemaVector3FieldIdY, 0.25f);
	Schema_AddFloat(scale, shovelerWorkerSchemaVector3FieldIdZ, 0.25f);
	Schema_AddBool(model, shovelerWorkerSchemaModelFieldIdVisible, true);
	Schema_AddBool(model, shovelerWorkerSchemaModelFieldIdEmitter, false);
	Schema_AddBool(model, shovelerWorkerSchemaModelFieldIdCastsShadow, true);
	Schema_AddEnum(model, shovelerWorkerSchemaModelFieldIdPolygonMode, shovelerWorkerSchemaPolygonModeFill);

	Worker_RequestId createEntityRequestId = Worker_Connection_SendCreateEntityRequest(
		context->connection,
		sizeof(cubeEntityComponentData) / sizeof(cubeEntityComponentData[0]),
		cubeEntityComponentData,
		/* entity_id */ NULL,
		/* timeout_millis */ NULL);
	if(createEntityRequestId == -1) {
		shovelerLogError("Failed to send create entity request.");
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "entity creation failure");
		return;
	}

	shovelerLogInfo("Sent create entity request %"PRId64"", createEntityRequestId);

	Worker_CommandResponse commandResponse;
	commandResponse.component_id = op->request.component_id;
	commandResponse.command_index = op->request.command_index;
	commandResponse.schema_type = Schema_CreateCommandResponse();

	int8_t result = Worker_Connection_SendCommandResponse(context->connection, op->request_id, &commandResponse);
	if(result == WORKER_RESULT_FAILURE) {
		shovelerLogError("Failed to send client spawn cube response.");
	}
}

static void onDigHoleRequest(ServerContext *context, const Worker_CommandRequestOp *op)
{
	const int numChunkColumns = 2 * halfMapWidth / chunkSize;
	const int numChunkRows = 2 * halfMapHeight / chunkSize;

	shovelerLogInfo("Received dig hole request from '%s'.", op->caller_worker_id);

	Schema_Object *requestObject = Schema_GetCommandRequestObject(op->request.schema_type);

	int64_t clientEntityId = Schema_GetEntityId(requestObject, shovelerWorkerSchemaDigHoleRequestFieldIdClient);
	Entity *clientEntity = g_hash_table_lookup(context->entities, &clientEntityId);
	if(clientEntity == NULL) {
		shovelerLogWarning("Received dig hole request from %s for unknown client entity %"PRId64", ignoring.", op->caller_worker_id, clientEntityId);
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "unknown client entity");
		return;
	}

	Schema_Object *positionObject = Schema_GetObject(requestObject, shovelerWorkerSchemaDigHoleRequestFieldIdPosition);
	if(positionObject == NULL) {
		shovelerLogWarning("Dig hole request doesn't contain position - ignoring.");
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "no position");
		return;
	}

	ShovelerVector3 requestPosition = shovelerVector3(
		Schema_GetFloat(positionObject, shovelerWorkerSchemaVector3FieldIdX),
		Schema_GetFloat(positionObject, shovelerWorkerSchemaVector3FieldIdY),
		Schema_GetFloat(positionObject, shovelerWorkerSchemaVector3FieldIdZ));

	ShovelerVector3 improbablePosition = remapPosition(&requestPosition, /* isTiles */ true);

	double x = improbablePosition.values[0];
	double z = improbablePosition.values[2];
	int chunkX, chunkZ, tileX, tileZ;
	worldToTile(x, z, &chunkX, &chunkZ, &tileX, &tileZ);

	if(chunkX < 0 || chunkX >= numChunkColumns || chunkZ < 0 || chunkZ >= numChunkRows || tileX < 0 || tileX >= chunkSize || tileZ < 0 || tileZ >= chunkSize) {
		shovelerLogWarning("Received dig hole request from %s for client entity %"PRId64" which is out of range at (%f, %f), ignoring.", op->caller_worker_id, clientEntityId, x, z);
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "out of range");
		return;
	}

	int64_t chunkBackgroundEntityId = getChunkBackgroundEntityId(chunkX, chunkZ);
	TilemapTiles *tiles = getChunkBackgroundTiles(context, chunkBackgroundEntityId);
	if(tiles == NULL) {
		shovelerLogError("Received dig hole request from %s for client entity %"PRId64", but failed to resolve chunk background entity %lld tilemap tiles.", op->caller_worker_id, clientEntityId, chunkBackgroundEntityId);
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "no background tilemap tiles");
		return;
	}

	char *tilesetColumn = &tiles->tilesetColumns->str[tileZ * chunkSize + tileX];
	char *tilesetRows = &tiles->tilesetRows->str[tileZ * chunkSize + tileX];
	char *tilesetIds = &tiles->tilesetIds->str[tileZ * chunkSize + tileX];
	if(*tilesetColumn > 2) {
		shovelerLogWarning("Received dig hole request from %s for client entity %"PRId64", but its current tile is not grass.", op->caller_worker_id, clientEntityId);
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "not grass");
		return;
	}

	*tilesetColumn = 6;
	*tilesetRows = 1;
	*tilesetIds = 2;

	Worker_ComponentUpdate tilemapTilesUpdate;
	tilemapTilesUpdate.component_id = shovelerWorkerSchemaComponentIdTilemapTiles;
	tilemapTilesUpdate.schema_type = Schema_CreateComponentUpdate();
	Schema_Object *tilemapTilesFields = Schema_GetComponentUpdateFields(tilemapTilesUpdate.schema_type);
	uint8_t *tilesetColumnsBuffer = Schema_AllocateBuffer(tilemapTilesFields, tiles->tilesetColumns->len);
	uint8_t *tilesetRowsBuffer = Schema_AllocateBuffer(tilemapTilesFields, tiles->tilesetRows->len);
	uint8_t *tilesetIdsBuffer = Schema_AllocateBuffer(tilemapTilesFields, tiles->tilesetIds->len);
	memcpy(tilesetColumnsBuffer, tiles->tilesetColumns->str, tiles->tilesetColumns->len);
	memcpy(tilesetRowsBuffer, tiles->tilesetRows->str, tiles->tilesetRows->len);
	memcpy(tilesetIdsBuffer, tiles->tilesetIds->str, tiles->tilesetIds->len);
	Schema_AddBytes(tilemapTilesFields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns, tilesetColumnsBuffer, tiles->tilesetColumns->len);
	Schema_AddBytes(tilemapTilesFields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows, tilesetRowsBuffer, tiles->tilesetRows->len);
	Schema_AddBytes(tilemapTilesFields, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds, tilesetIdsBuffer, tiles->tilesetIds->len);

	Worker_Connection_SendComponentUpdate(context->connection, chunkBackgroundEntityId, &tilemapTilesUpdate, /* update_parameters */ NULL);

	Worker_CommandResponse commandResponse;
	commandResponse.component_id = op->request.component_id;
	commandResponse.command_index = op->request.command_index;
	commandResponse.schema_type = Schema_CreateCommandResponse();

	int8_t result = Worker_Connection_SendCommandResponse(context->connection, op->request_id, &commandResponse);
	if(result == WORKER_RESULT_FAILURE) {
		shovelerLogError("Failed to send dig hole response.");
	}
}

static void onUpdateResourceRequest(ServerContext *context, const Worker_CommandRequestOp *op)
{
	shovelerLogInfo("Received update resource request from '%s'.", op->caller_worker_id);

	Schema_Object *requestObject = Schema_GetCommandRequestObject(op->request.schema_type);

	int64_t resourceEntityId = Schema_GetEntityId(requestObject, shovelerWorkerSchemaUpdateResourceRequestFieldIdResource);
	uint32_t contentCount = Schema_GetBytesCount(requestObject, shovelerWorkerSchemaUpdateResourceRequestFieldIdContent);
	if(contentCount == 0) {
		shovelerLogWarning("Received update resource request from %s for resource entity %"PRId64", but no content was provided.", op->caller_worker_id, resourceEntityId);
		Worker_Connection_SendCommandFailure(context->connection, op->request_id, "no content");
		return;
	}

	uint32_t contentLength = Schema_GetBytesLength(requestObject, shovelerWorkerSchemaUpdateResourceRequestFieldIdContent);
	const uint8_t *contentBytes = Schema_GetBytes(requestObject, shovelerWorkerSchemaUpdateResourceRequestFieldIdContent);

	Worker_ComponentUpdate resourceUpdate;
	resourceUpdate.component_id = shovelerWorkerSchemaComponentIdResource;
	resourceUpdate.schema_type = Schema_CreateComponentUpdate();
	Schema_Object *resourceFields = Schema_GetComponentUpdateFields(resourceUpdate.schema_type);
	uint8_t *resourceBuffer = Schema_AllocateBuffer(resourceFields, contentLength);
	memcpy(resourceBuffer, contentBytes, contentLength);
	Schema_AddBytes(resourceFields, shovelerWorkerSchemaResourceFieldIdBuffer, resourceBuffer, contentLength);

	Worker_Connection_SendComponentUpdate(context->connection, resourceEntityId, &resourceUpdate, /* update_parameters */ NULL);

	Worker_CommandResponse commandResponse;
	commandResponse.component_id = op->request.component_id;
	commandResponse.command_index = op->request.command_index;
	commandResponse.schema_type = Schema_CreateCommandResponse();

	int8_t result = Worker_Connection_SendCommandResponse(context->connection, op->request_id, &commandResponse);
	if(result == WORKER_RESULT_FAILURE) {
		shovelerLogError("Failed to send update resource response.");
	}
}

static Client *getOrCreateClient(ServerContext *context, int64_t entityId)
{
	Client *client = g_hash_table_lookup(context->clients, &entityId);
	if(client == NULL) {
		client = malloc(sizeof(Client));
		client->entityId = entityId;
		client->workerId = NULL;
		client->lastPong = 0;

		g_hash_table_insert(context->clients, &client->entityId, client);

		shovelerLogInfo("Starting to track client with entity ID %"PRId64".", entityId);
	}

	return client;
}

static ShovelerVector3 getNewPlayerPosition(ServerContext *context, Schema_Object *requestObject)
{
	if(context->configuration.gameType == SHOVELER_WORKER_GAME_TYPE_LIGHTS) {
		return shovelerVector3(0.0f, 5.0f, 0.0f);
	}

	for(int i = 0; i < numPlayerPositionAttempts; i++) {
		int minX = 9;
		int minZ = 9;
		int sizeX = 2;
		int sizeZ = 2;
		if(Schema_GetObjectCount(requestObject, shovelerWorkerSchemaCreateClientEntityRequestFieldIdStartingChunkRegion) != 0) {
			Schema_Object *startingChunkRegion = Schema_GetObject(requestObject, shovelerWorkerSchemaCreateClientEntityRequestFieldIdStartingChunkRegion);

			minX = Schema_GetInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdMinX);
			minZ = Schema_GetInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdMaxX);
			sizeX = Schema_GetInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdSizeX);
			sizeZ = Schema_GetInt32(startingChunkRegion, shovelerWorkerSchemaChunkRegionFieldIdSizeZ);
			shovelerLogInfo("Overriding starting chunk region to min (%d, %d) and size (%d, %d).", minX, minZ, sizeX, sizeZ);
		}

		int startingChunkX = minX + (rand() % sizeX);
		int startingChunkZ = minZ + (rand() % sizeZ);

		int64_t backgroundChunkEntityId = getChunkBackgroundEntityId(startingChunkX, startingChunkZ);

		TilemapTiles *tilemapTiles = getChunkBackgroundTiles(context, backgroundChunkEntityId);
		if(!tilemapTiles) {
			shovelerLogWarning("Failed to find chunk background tiles on entity %"PRId64".", backgroundChunkEntityId);
			continue;
		}

		int startingTileX = rand() % 10;
		int startingTileZ = rand() % 10;
		unsigned char tilesetColumn = tilemapTiles->tilesetColumns->str[startingTileZ * chunkSize + startingTileX];
		unsigned char tilesetRows = tilemapTiles->tilesetRows->str[startingTileZ * chunkSize + startingTileX];
		unsigned char tilesetIds = tilemapTiles->tilesetIds->str[startingTileZ * chunkSize + startingTileX];
		if(tilesetColumn > 2) { // not grass
			continue;
		}

		ShovelerVector2 worldPosition2 = tileToWorld(startingChunkX, startingChunkZ, startingTileX, startingTileZ);
		shovelerLogInfo("Rolled new player position in tile (%d, %d) of chunk (%d, %d) after %d iterations: (%.2f, %.2f)", startingTileX, startingTileZ, startingChunkX, startingChunkZ, i + 1, worldPosition2.values[0], worldPosition2.values[1]);

		int chunkX, chunkZ, tileX, tileZ;
		worldToTile(worldPosition2.values[0] + 0.5, worldPosition2.values[1] + 0.5, &chunkX, &chunkZ, &tileX, &tileZ);
		shovelerLogInfo("Back translation: tile (%d, %d) chunk (%d, %d)", tileX, tileZ, chunkX, chunkZ);

		return shovelerVector3(worldPosition2.values[0] + 0.5f, 5.0f, worldPosition2.values[1] + 0.5f);
	}

	shovelerLogInfo("Using default position after %d failed attempts to roll new player position.", numPlayerPositionAttempts);
	return shovelerVector3(0.5f, 5.0f, 0.5f);
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

static TilemapTiles *getChunkBackgroundTiles(ServerContext *context, int64_t chunkBackgroundEntityId)
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

static const char *getSpecificWorkerAttribute(const Worker_WorkerAttributes *workerAttributes)
{
	for(uint32_t i = 0; i < workerAttributes->attribute_count; i++) {
		const char *attribute = workerAttributes->attributes[i];

		if(g_str_has_prefix(attribute, "workerId:")) {
			return attribute;
		}
	}

	return "";
}

static ShovelerVector3 remapImprobablePosition(const ShovelerVector3 *coordinates, bool isTiles)
{
	if(isTiles) {
		return shovelerVector3(
			coordinates->values[0],
			coordinates->values[2],
			coordinates->values[1]);
	} else {
		return shovelerVector3(
			-coordinates->values[0],
			coordinates->values[1],
			coordinates->values[2]);
	}
}

static ShovelerVector3 remapPosition(const ShovelerVector3 *coordinates, bool isTiles)
{
	if(isTiles) {
		return shovelerVector3(
			coordinates->values[0],
			coordinates->values[2],
			coordinates->values[1]);
	} else {
		return shovelerVector3(
			-coordinates->values[0],
			coordinates->values[1],
			coordinates->values[2]);
	}
}

static ShovelerVector4 colorFromHsv(float h, float s, float v)
{
	ShovelerColor colorRgb = shovelerColorFromHsv(h, s, v);
	ShovelerVector3 colorFloat = shovelerColorToVector3(colorRgb);

	return shovelerVector4(colorFloat.values[0], colorFloat.values[1], colorFloat.values[2], 1.0f);
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

static void freeClient(void *clientPointer)
{
	Client *client = clientPointer;
	free(client->workerId);
	free(client);
}
