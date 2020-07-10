#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <unordered_set>

#include <improbable/standard_library.h>
#include <improbable/view.h>
#include <improbable/worker.h>
#include <shoveler.h>
#include <set>

extern "C" {
#include <glib.h>

#include <shoveler/color.h>
#include <shoveler/executor.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
}

using improbable::ComponentInterest;
using improbable::Coordinates;
using improbable::EntityAcl;
using improbable::EntityAclData;
using improbable::Interest;
using improbable::Metadata;
using improbable::Persistence;
using ImprobablePosition = improbable::Position;
using improbable::WorkerAttributeSet;
using improbable::WorkerRequirementSet;
using shoveler::Bootstrap;
using shoveler::Canvas;
using shoveler::Client;
using shoveler::ClientHeartbeatPing;
using shoveler::ClientHeartbeatPong;
using shoveler::ClientInfo;
using shoveler::ClientInfoData;
using shoveler::CoordinateMapping;
using shoveler::CreateClientEntityRequest;
using shoveler::CreateClientEntityResponse;
using shoveler::Drawable;
using shoveler::DrawableType;
using shoveler::Light;
using shoveler::LightType;
using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Model;
using shoveler::PolygonMode;
using shoveler::Position;
using shoveler::PositionType;
using shoveler::PositionData;
using shoveler::Resource;
using shoveler::ResourceData;
using shoveler::Sprite;
using shoveler::TilemapTiles;
using shoveler::TilemapTilesData;
using shoveler::TileSprite;
using shoveler::TileSpriteAnimation;
using shoveler::Vector4;
using worker::Authority;
using worker::CreateEntityRequest;
using worker::EntityId;
using worker::List;
using worker::None;
using worker::Option;
using worker::RequestId;
using worker::Result;

using CreateClientEntity = shoveler::Bootstrap::Commands::CreateClientEntity;
using ClientSpawnCube = shoveler::Bootstrap::Commands::ClientSpawnCube;
using DigHole = shoveler::Bootstrap::Commands::DigHole;
using UpdateResource = shoveler::Bootstrap::Commands::UpdateResource;
using Query = ComponentInterest::Query;
using QueryConstraint = ComponentInterest::QueryConstraint;

struct TilesData {
	std::string tilesetColumns;
	std::string tilesetRows;
	std::string tilesetIds;
};

struct ServerContext {
	worker::Connection *connection;
	worker::View *view;
	std::unordered_set<worker::EntityId> authoritativeClients;
	std::unordered_map<worker::EntityId, std::string> clientWorkerIds;
};

const int halfMapWidth = 100;
const int halfMapHeight = 100;
const int chunkSize = 10;
const EntityId firstChunkEntityId = 12;
const int numChunkColumns = 2 * halfMapWidth / chunkSize;
const int numChunkRows = 2 * halfMapHeight / chunkSize;
const int numPlayerPositionAttempts = 10;
const int64_t maxHeartbeatTimeoutMs = 5000;

static void onLogMessage(const worker::LogData& logData);
static void clientCleanupTick(void *contextPointer);
static Vector4 colorFromHsv(float h, float s, float v);
static ShovelerVector2 tileToWorld(int chunkX, int chunkZ, int tileX, int tileZ);
static void worldToTile(double x, double z, int &chunkX, int &chunkZ, int &tileX, int &tileZ);
static EntityId getChunkBackgroundEntityId(int chunkX, int chunkZ);
static Option<TilesData> getChunkBackgroundTiles(worker::Connection &connection, worker::View &view, EntityId chunkBackgroundEntityId);
static Coordinates getNewPlayerPosition(worker::Connection &connection, worker::View &view, const CreateClientEntityRequest& request);
static WorkerRequirementSet getSpecificWorkerRequirementSet(worker::List<std::string> attributeSet);
static ShovelerVector3 remapImprobablePosition(const Coordinates& coordinates, bool isTiles);
static Coordinates remapPosition(const ShovelerVector3& coordinates, bool isTiles);

int main(int argc, char **argv) {
	if(argc != 5) {
		return 1;
	}

	srand(time(NULL));

	worker::LogsinkParameters logsinkParameters;
	logsinkParameters.Type = worker::LogsinkType::kCallback;
	logsinkParameters.Callback = onLogMessage;
	logsinkParameters.FilterParameters.Categories = worker::LogCategory::kNetworkStatus | worker::LogCategory::kLogin;
	logsinkParameters.FilterParameters.Level = worker::LogLevel::kInfo;

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "ShovelerServer";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kModularTcp;
	parameters.Network.ModularTcp.SecurityType = worker::NetworkSecurityType::kInsecure;
	parameters.Network.UseExternalIp = false;
	parameters.Logsinks = {logsinkParameters};
	parameters.EnableLoggingAtStartup = true;

	const std::string workerId = argv[1];
	const std::string hostname = argv[2];
	const std::uint16_t port = static_cast<std::uint16_t>(std::stoi(argv[3]));
	const std::string logFileLocation = argv[4];
	const int tickRateHz = 100;
	const int clientCleanupTickRateHz = 2;
	const int64_t maxHeartbeatTimeoutMs = 5000;
	EntityId cubeDrawableEntityId = 2;
	EntityId pointDrawableEntityId = 4;
	const EntityId characterAnimationTilesetEntityId = 5;
	const EntityId character2AnimationTilesetEntityId = 6;
	const EntityId character3AnimationTilesetEntityId = 7;
	const EntityId character4AnimationTilesetEntityId = 8;
	const EntityId canvasEntityId = 9;
	int characterCounter = 0;

	FILE *logFile = fopen(logFileLocation.c_str(), "w+");
	if(logFile == NULL) {
		std::cerr << "Failed to open output log file at " << logFileLocation << ": " << strerror(errno) << std::endl;
		return 1;
	}

	shovelerLogInit("shoveler-spatialos/", SHOVELER_LOG_LEVEL_INFO_UP, logFile);
	shovelerLogInfo("Connecting as worker %s to %s:%d.", workerId.c_str(), hostname.c_str(), port);

	auto components = worker::Components<
		Bootstrap,
		Canvas,
		Client,
		ClientHeartbeatPing,
		ClientHeartbeatPong,
		ClientInfo,
		Drawable,
		EntityAcl,
		ImprobablePosition,
		Interest,
		Light,
		Material,
		Metadata,
		Model,
		Persistence,
		Position,
		Resource,
		Sprite,
		TilemapTiles,
		TileSprite,
		TileSpriteAnimation>{};

	worker::Connection connection = worker::Connection::ConnectAsync(components, hostname, port, workerId, parameters).Get();
	bool disconnected = false;
	worker::View view{components};
	worker::Dispatcher &dispatcher = view;

	ServerContext context;
	context.connection = &connection;
	context.view = &view;
	context.authoritativeClients = {};

	const Option<std::string>& flagOption = connection.GetWorkerFlag("game_type");
	bool isTiles = flagOption && *flagOption == "tiles";

	ShovelerExecutor *tickExecutor = shovelerExecutorCreateDirect();
	std::map<worker::EntityId, std::pair<std::string, int64_t>> lastHeartbeatMicrosMap;

	std::set<worker::EntityId> connectedClients;

	dispatcher.OnDisconnect([&](const worker::DisconnectOp &op) {
		shovelerLogError("Disconnected from SpatialOS: %s", op.Reason.c_str());
		disconnected = true;
	});

	dispatcher.OnMetrics([&](const worker::MetricsOp &op) {
		auto metrics = op.Metrics;
		connection.SendMetrics(metrics);
	});

	dispatcher.OnAddEntity([&](const worker::AddEntityOp &op) {
		shovelerLogInfo("Adding entity %lld.", op.EntityId);
	});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp &op) {
		shovelerLogInfo("Removing entity %lld.", op.EntityId);
	});

	dispatcher.OnAddComponent<Bootstrap>([&](const worker::AddComponentOp<Bootstrap> &op) {
		shovelerLogInfo("Adding bootstrap to entity %lld.", op.EntityId);
	});

	dispatcher.OnComponentUpdate<Bootstrap>([&](const worker::ComponentUpdateOp<Bootstrap> &op) {
		shovelerLogInfo("Updating bootstrap for entity %lld.", op.EntityId);
	});

	dispatcher.OnRemoveComponent<Bootstrap>([&](const worker::RemoveComponentOp &op) {
		shovelerLogInfo("Removing bootstrap from entity %lld.", op.EntityId);
	});

	dispatcher.OnAuthorityChange<Bootstrap>([&](const worker::AuthorityChangeOp &op) {
		shovelerLogInfo("Authority change to %d for entity %lld.", op.Authority, op.EntityId);
	});

	dispatcher.OnAuthorityChange<ClientHeartbeatPong>([&](const worker::AuthorityChangeOp &op) {
		if(op.Authority == Authority::kAuthoritative) {
			context.authoritativeClients.insert(op.EntityId);
			shovelerLogInfo("Added authoritative client %lld.", op.EntityId);
		} else if(op.Authority == Authority::kNotAuthoritative) {
			context.authoritativeClients.erase(op.EntityId);
			shovelerLogInfo("Removed authoritative client %lld.", op.EntityId);
		}
	});

	dispatcher.OnAddComponent<ClientInfo>([&](const worker::AddComponentOp<ClientInfo> &op) {
		context.clientWorkerIds[op.EntityId] = op.Data.worker_id();
	});

	dispatcher.OnRemoveComponent<ClientInfo>([&](const worker::RemoveComponentOp &op) {
		context.clientWorkerIds.erase(op.EntityId);
	});

	dispatcher.OnComponentUpdate<ClientHeartbeatPing>([&](const worker::ComponentUpdateOp<ClientHeartbeatPing> &op) {
		if(!op.Update.last_updated_time()) {
			return;
		}

		const auto &authorityQuery = view.ComponentAuthority.find(op.EntityId);
		if(authorityQuery == view.ComponentAuthority.end()) {
			return;
		}

		const auto &componentAuthorityQuery = authorityQuery->second.find(ClientHeartbeatPong::ComponentId);
		if(componentAuthorityQuery == authorityQuery->second.end()) {
			return;
		}

		if(componentAuthorityQuery->second != Authority::kAuthoritative) {
			return;
		}

		ClientHeartbeatPong::Update heartbeatPongUpdate;
		heartbeatPongUpdate.set_last_updated_time(*op.Update.last_updated_time());
		connection.SendComponentUpdate<ClientHeartbeatPong>(op.EntityId, heartbeatPongUpdate);

		shovelerLogTrace("Reflected client %lld heartbeat pong update.", op.EntityId);
	});

	dispatcher.OnCommandRequest<CreateClientEntity>([&](const worker::CommandRequestOp<CreateClientEntity> &op) {
		shovelerLogInfo("Received create client entity request from: %s", op.CallerWorkerId.c_str());

		Coordinates playerImprobablePosition = getNewPlayerPosition(connection, view, op.Request);
		ShovelerVector3 playerPosition = remapImprobablePosition(playerImprobablePosition, isTiles);

		int64_t initialHeartbeat = g_get_monotonic_time() + 5 * 1000 * maxHeartbeatTimeoutMs;

		worker::Entity clientEntity;
		clientEntity.Add<Metadata>({"client"});
		clientEntity.Add<Client>({});
		clientEntity.Add<ClientHeartbeatPing>({initialHeartbeat});
		clientEntity.Add<ClientHeartbeatPong>({initialHeartbeat});
		clientEntity.Add<Persistence>({});
		clientEntity.Add<ImprobablePosition>(playerImprobablePosition);
		clientEntity.Add<Position>({PositionType::ABSOLUTE, {playerPosition.values[0], playerPosition.values[1], playerPosition.values[2]}, {}});

		QueryConstraint relativeConstraint;
		relativeConstraint.set_relative_box_constraint({{{20.5, 9999, 20.5}}});
		Query relativeQuery;
		relativeQuery.set_constraint(relativeConstraint);
		relativeQuery.set_full_snapshot_result({true});
		ComponentInterest interest{{relativeQuery}};
		clientEntity.Add<Interest>({{{Client::ComponentId, interest}}});

		WorkerAttributeSet clientAttributeSet({"client"});
		WorkerAttributeSet serverAttributeSet({"server"});
		WorkerRequirementSet specificClientRequirementSet(getSpecificWorkerRequirementSet(op.CallerAttributeSet));
		WorkerRequirementSet serverRequirementSet({serverAttributeSet});
		WorkerRequirementSet clientAndServerRequirementSet({clientAttributeSet, serverAttributeSet});
		worker::Map<std::uint32_t, WorkerRequirementSet> clientEntityComponentAclMap;
		clientEntityComponentAclMap.insert({{Client::ComponentId, specificClientRequirementSet}});
		clientEntityComponentAclMap.insert({{ClientHeartbeatPing::ComponentId, specificClientRequirementSet}});
		clientEntityComponentAclMap.insert({{ClientHeartbeatPong::ComponentId, serverRequirementSet}});
		clientEntityComponentAclMap.insert({{Interest::ComponentId, specificClientRequirementSet}});
		clientEntityComponentAclMap.insert({{ImprobablePosition::ComponentId, specificClientRequirementSet}});
		clientEntityComponentAclMap.insert({{Position::ComponentId, specificClientRequirementSet}});
		EntityAclData clientEntityAclData(clientAndServerRequirementSet, clientEntityComponentAclMap);
		clientEntity.Add<EntityAcl>(clientEntityAclData);

		const Option<std::string> &flagOption = connection.GetWorkerFlag("game_type");
		float hue = 0.0f;
		float saturation = 0.0f;
		if(flagOption && *flagOption == "tiles") {
			int modulo = characterCounter++ % 4;
			worker::EntityId tilesetEntityId = characterAnimationTilesetEntityId;
			if(modulo == 1) {
				tilesetEntityId = character2AnimationTilesetEntityId;
			} else if(modulo == 2) {
				tilesetEntityId = character3AnimationTilesetEntityId;
			} else if(modulo == 3) {
				tilesetEntityId = character4AnimationTilesetEntityId;
			}

			clientEntity.Add<Sprite>({0, CoordinateMapping::POSITIVE_X, CoordinateMapping::POSITIVE_Y, false, canvasEntityId, 1, {1.0f, 1.0f}, {0}, {}, {}});
			clientEntity.Add<TileSprite>({canvasEntityId, tilesetEntityId, 0, 0});
			clientEntity.Add<TileSpriteAnimation>({0, 0, CoordinateMapping::POSITIVE_X, CoordinateMapping::POSITIVE_Y, 0.5});
		} else {
			hue = (float) rand() / RAND_MAX;
			saturation = 0.5f + 0.5f * ((float) rand() / RAND_MAX);
			Vector4 playerParticleColor = colorFromHsv(hue, saturation, 0.9f);
			Vector4 playerLightColor = colorFromHsv(hue, saturation, 0.1f);

			clientEntity.Add<Material>({MaterialType::PARTICLE, {}, {}, {}, {}, {}, {playerParticleColor}, {}, {}});
			clientEntity.Add<Model>({0, pointDrawableEntityId, 0, {0.0f, 0.0f, 0.0f}, {0.1f, 0.1f, 0.0f}, true, true, false, PolygonMode::FILL});
			clientEntity.Add<Light>({0, LightType::POINT, 1024, 1024, 1, 0.01f, 80.0f, {playerLightColor.x(), playerLightColor.y(), playerLightColor.z()}});
		}

		clientEntity.Add<ClientInfo>({op.CallerWorkerId, hue, saturation});

		Result<RequestId<CreateEntityRequest>> createEntityRequestId = connection.SendCreateEntityRequest(clientEntity, {}, {});
		if(!createEntityRequestId) {
			shovelerLogError("Failed to send create entity request: %s", createEntityRequestId.GetErrorMessage().c_str());
			return;
		}

		shovelerLogInfo("Sent create entity request %lld.", createEntityRequestId->Id);

		Result<None> commandResponseSent = connection.SendCommandResponse<CreateClientEntity>(op.RequestId, {});
		if(!commandResponseSent) {
			shovelerLogError("Failed to send create client entity command %lld response: %s", op.RequestId.Id, commandResponseSent.GetErrorMessage().c_str());
		}
	});

	dispatcher.OnCommandRequest<ClientSpawnCube>([&](const worker::CommandRequestOp<ClientSpawnCube> &op) {
		worker::EntityId clientEntityId = op.Request.client();
		shovelerLogInfo("Received client spawn cube from %s for client entity %lld in direction (%.2f, %.2f, %.2f).", op.CallerWorkerId.c_str(), clientEntityId, op.Request.direction().x(), op.Request.direction().y(), op.Request.direction().z());

		worker::Map<worker::EntityId, worker::Entity>::const_iterator entityQuery = view.Entities.find(clientEntityId);
		if(entityQuery == view.Entities.end()) {
			shovelerLogWarning("Received client spawn cube from %s for unknown client entity %lld, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}

		const worker::Entity &clientEntity = entityQuery->second;
		worker::Option<const ClientInfoData &> clientInfoComponent = clientEntity.Get<ClientInfo>();
		if(!clientInfoComponent) {
			shovelerLogWarning("Received client spawn cube from %s for client entity %lld without client info component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}
		worker::Option<const PositionData &> positionComponent = clientEntity.Get<Position>();
		if(!positionComponent) {
			shovelerLogWarning("Received client spawn cube from %s for client entity %lld without position component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}

		ShovelerVector3 normalizedDirection = shovelerVector3Normalize(shovelerVector3(op.Request.direction().x(), op.Request.direction().y(), op.Request.direction().z()));
		ShovelerVector3 cubePosition = shovelerVector3LinearCombination(1.0f, shovelerVector3(positionComponent->coordinates().x(), positionComponent->coordinates().y(), positionComponent->coordinates().z()), 0.5f, normalizedDirection);

		Coordinates cubeImprobablePosition = remapPosition(cubePosition, isTiles);

		Vector4 cubeColor = colorFromHsv(clientInfoComponent->color_hue(), clientInfoComponent->color_saturation(), 0.7f);

		worker::Entity cubeEntity;
		cubeEntity.Add<Metadata>({"cube"});
		cubeEntity.Add<Persistence>({});
		cubeEntity.Add<ImprobablePosition>({cubeImprobablePosition});
		cubeEntity.Add<Position>({PositionType::ABSOLUTE, {cubePosition.values[0], cubePosition.values[1], cubePosition.values[2]}, {}});
		cubeEntity.Add<Material>({MaterialType::COLOR, {}, {}, {}, {}, {}, cubeColor, {}, {}});
		cubeEntity.Add<Model>({0, cubeDrawableEntityId, 0, op.Request.rotation(), {0.25f, 0.25f, 0.25f}, true, false, true, PolygonMode::FILL});

		WorkerAttributeSet clientAttributeSet({"client"});
		WorkerRequirementSet clientRequirementSet({clientAttributeSet});
		cubeEntity.Add<EntityAcl>({clientRequirementSet, {}});

		Result<RequestId<CreateEntityRequest>> createEntityRequestId = connection.SendCreateEntityRequest(cubeEntity, {}, {});
		if(!createEntityRequestId) {
			shovelerLogError("Failed to send create entity request: %s", createEntityRequestId.GetErrorMessage().c_str());
			return;
		}

		Result<None> commandResponseSent = connection.SendCommandResponse<ClientSpawnCube>(op.RequestId, {});
		if(!commandResponseSent) {
			shovelerLogError("Failed to send client spawn cube command %lld response: %s", op.RequestId.Id, commandResponseSent.GetErrorMessage().c_str());
			return;
		}
	});

	dispatcher.OnCommandRequest<DigHole>([&](const worker::CommandRequestOp<DigHole> &op) {
		worker::EntityId clientEntityId = op.Request.client();
		shovelerLogInfo("Received dig hole request from %s for client entity %lld.", op.CallerWorkerId.c_str(), clientEntityId);

		worker::Map<worker::EntityId, worker::Entity>::const_iterator entityQuery = view.Entities.find(clientEntityId);
		if(entityQuery == view.Entities.end()) {
			shovelerLogWarning("Received dig hole request from %s for unknown client entity %lld, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "unknown client entity");
			return;
		}

		const worker::Entity &clientEntity = entityQuery->second;
		worker::Option<const PositionData &> positionComponent = clientEntity.Get<Position>();
		if(!positionComponent) {
			shovelerLogWarning("Received dig hole request from %s for client entity %lld without position component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "client entity without position");
			return;
		}

		Coordinates improbablePosition = remapPosition(
			shovelerVector3(positionComponent->coordinates().x(), positionComponent->coordinates().y(), positionComponent->coordinates().z()),
			isTiles);

		double x = improbablePosition.x();
		double z = improbablePosition.z();
		int chunkX, chunkZ, tileX, tileZ;
		worldToTile(x, z, chunkX, chunkZ, tileX, tileZ);

		if(chunkX < 0 || chunkX >= numChunkColumns || chunkZ < 0 || chunkZ >= numChunkRows || tileX < 0 || tileX >= chunkSize || tileZ < 0 || tileZ >= chunkSize) {
			shovelerLogWarning("Received dig hole request from %s for client entity %lld which is out of range at (%f, %f), ignoring.", op.CallerWorkerId.c_str(), clientEntityId, x, z);
			connection.SendCommandFailure<DigHole>(op.RequestId, "out of range");
			return;
		}

		worker::EntityId chunkBackgroundEntityId = getChunkBackgroundEntityId(chunkX, chunkZ);
		Option<TilesData> tiles = getChunkBackgroundTiles(connection, view, chunkBackgroundEntityId);
		if(!tiles) {
			shovelerLogError("Received dig hole request from %s for client entity %lld, but failed to resolve chunk background entity %lld tilemap tiles.", op.CallerWorkerId.c_str(), clientEntityId, chunkBackgroundEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "failed to resolve chunk background entity tilemap tiles");
			return;
		}

		char& tilesetColumn = tiles->tilesetColumns[tileZ * chunkSize + tileX];
		char& tilesetRows = tiles->tilesetRows[tileZ * chunkSize + tileX];
		char& tilesetIds = tiles->tilesetIds[tileZ * chunkSize + tileX];
		if(tilesetColumn > 2) {
			shovelerLogWarning("Received dig hole request from %s for client entity %lld, but its current tile is not grass.", op.CallerWorkerId.c_str(), clientEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "tile isn't grass");
			return;
		}

		tilesetColumn = 6;
		tilesetRows = 1;
		tilesetIds = 2;

		TilemapTiles::Update tilemapTilesUpdate;
		tilemapTilesUpdate.set_tileset_columns(tiles->tilesetColumns);
		tilemapTilesUpdate.set_tileset_rows(tiles->tilesetRows);
		tilemapTilesUpdate.set_tileset_ids(tiles->tilesetIds);
		connection.SendComponentUpdate<TilemapTiles>(chunkBackgroundEntityId, tilemapTilesUpdate);

		Result<None> commandResponseSent = connection.SendCommandResponse<DigHole>(op.RequestId, {});
		if(!commandResponseSent) {
			shovelerLogError("Failed to send dig hole command %lld response: %s", op.RequestId.Id, commandResponseSent.GetErrorMessage().c_str());
			return;
		}
	});

	dispatcher.OnCommandRequest<UpdateResource>([&](const worker::CommandRequestOp<UpdateResource> &op) {
		worker::EntityId resourceEntityId = op.Request.resource();
		shovelerLogInfo("Received update request from %s for resource entity %lld.", op.CallerWorkerId.c_str(), resourceEntityId);

		worker::Map<worker::EntityId, worker::Entity>::const_iterator entityQuery = view.Entities.find(resourceEntityId);
		if(entityQuery == view.Entities.end()) {
			shovelerLogWarning("Received update request from %s for unknown resource entity %lld, ignoring.", op.CallerWorkerId.c_str(), resourceEntityId);
			connection.SendCommandFailure<UpdateResource>(op.RequestId, "unknown resource entity");
			return;
		}

		const worker::Entity &resourceEntity = entityQuery->second;
		worker::Option<const ResourceData &> resourceComponent = resourceEntity.Get<Resource>();
		if(!resourceComponent) {
			shovelerLogWarning("Received update request from %s for resource entity %lld without resource component, ignoring.", op.CallerWorkerId.c_str(), resourceEntityId);
			connection.SendCommandFailure<UpdateResource>(op.RequestId, "client entity without position");
			return;
		}

		Resource::Update resourceUpdate;
		resourceUpdate.set_buffer(op.Request.content());
		connection.SendComponentUpdate<Resource>(resourceEntityId, resourceUpdate);

		Result<None> commandResponseSent = connection.SendCommandResponse<UpdateResource>(op.RequestId, {});
		if(!commandResponseSent) {
			shovelerLogError("Failed to send update resource command %lld response: %s", op.RequestId.Id, commandResponseSent.GetErrorMessage().c_str());
			return;
		}
	});

	dispatcher.OnCreateEntityResponse([&](const worker::CreateEntityResponseOp &op) {
		shovelerLogInfo("Received create entity response for request %lld with status code %d: %s", op.RequestId.Id, op.StatusCode, op.Message.c_str());
	});

	dispatcher.OnDeleteEntityResponse([&](const worker::DeleteEntityResponseOp &op) {
		shovelerLogInfo("Received delete entity response for request %lld with status code %d: %s", op.RequestId.Id, op.StatusCode, op.Message.c_str());
	});

	uint32_t clientCleanupTickPeriod = (uint32_t) (1000.0 / (double) clientCleanupTickRateHz);
	shovelerExecutorSchedulePeriodic(tickExecutor, 0, clientCleanupTickPeriod, clientCleanupTick, &context);

	uint32_t executorTickPeriod = (uint32_t) (1000.0 / (double) tickRateHz);
	while(!disconnected) {
		dispatcher.Process(connection.GetOpList(executorTickPeriod));
		shovelerExecutorUpdateNow(tickExecutor);
	}

	shovelerExecutorFree(tickExecutor);
}

static void onLogMessage(const worker::LogData& logData)
{
	switch(logData.Level) {
		case worker::LogLevel::kDebug:
			shovelerLogTrace("[Worker SDK] %s", logData.Content.c_str());
			break;
		case worker::LogLevel::kInfo:
			shovelerLogInfo("[Worker SDK] %s", logData.Content.c_str());
			break;
		case worker::LogLevel::kWarn:
			shovelerLogWarning("[Worker SDK] %s", logData.Content.c_str());
			break;
		case worker::LogLevel::kError:
			shovelerLogError("[Worker SDK] %s", logData.Content.c_str());
			break;
		case worker::LogLevel::kFatal:
			shovelerLogError("[Worker SDK] [FATAL] %s", logData.Content.c_str());
			break;
	}
}

static void clientCleanupTick(void *contextPointer) {
	ServerContext *context = (ServerContext *) contextPointer;

	int64_t now = g_get_monotonic_time();
	for(const auto& authoritativeClient : context->authoritativeClients) {
		auto entityQuery = context->view->Entities.find(authoritativeClient);
		if(entityQuery == context->view->Entities.end()) {
			continue;
		}
		auto& entity = entityQuery->second;

		auto pongData = entity.Get<ClientHeartbeatPong>();
		if(!pongData) {
			continue;
		}

		int64_t last = pongData->last_updated_time();
		int64_t diff = now - last;
		if(diff > 1000 * maxHeartbeatTimeoutMs) {
			worker::RequestId<worker::DeleteEntityRequest> deleteEntityRequestId = context->connection->SendDeleteEntityRequest(authoritativeClient, {});

			std::string clientWorkerId = "(unknown)";
			auto clientWorkerIdQuery = context->clientWorkerIds.find(authoritativeClient);
			if(clientWorkerIdQuery != context->clientWorkerIds.end()) {
				clientWorkerId = clientWorkerIdQuery->second;
			}

			shovelerLogWarning("Sent remove client entity %lld request %lld of worker %s because it exceeded the maximum heartbeat timeout of %lldms.", authoritativeClient, deleteEntityRequestId.Id, clientWorkerId.c_str(), maxHeartbeatTimeoutMs);
		}
	}
}

static Vector4 colorFromHsv(float h, float s, float v) {
	ShovelerColor colorRgb = shovelerColorFromHsv(h, s, v);
	ShovelerVector3 colorFloat = shovelerColorToVector3(colorRgb);

	return Vector4{colorFloat.values[0], colorFloat.values[1], colorFloat.values[2], 1.0f};
}

static ShovelerVector2 tileToWorld(int chunkX, int chunkZ, int tileX, int tileZ) {
	return shovelerVector2(-halfMapWidth + chunkX * chunkSize + tileX, -halfMapHeight + chunkZ * chunkSize + tileZ);
}

static void worldToTile(double x, double z, int &chunkX, int &chunkZ, int &tileX, int &tileZ) {
	double diffX = x + halfMapWidth;
	double diffZ = z + halfMapHeight;

	chunkX = (int) floor(diffX / chunkSize);
	chunkZ = (int) floor(diffZ / chunkSize);

	tileX = (int) floor(diffX - chunkX * chunkSize);
	tileZ = (int) floor(diffZ - chunkZ * chunkSize);
}

static EntityId getChunkBackgroundEntityId(int chunkX, int chunkZ) {
	if(chunkX < 0 || chunkX >= numChunkColumns || chunkZ < 0 || chunkZ >= numChunkRows) {
		shovelerLogWarning("Cannot resolve chunk background entity id for out of range chunk at (%d, %d).", chunkX, chunkZ);
		return 0;
	}

	return firstChunkEntityId + 3 * chunkX * numChunkColumns + 3 * chunkZ;
}

static Option<TilesData> getChunkBackgroundTiles(worker::Connection &connection, worker::View &view, EntityId chunkBackgroundEntityId) {
	worker::Map<worker::EntityId, worker::Entity>::const_iterator chunkBackgroundEntityQuery = view.Entities.find(chunkBackgroundEntityId);
	if(chunkBackgroundEntityQuery == view.Entities.end()) {
		shovelerLogWarning("Chunk background entity %lld is not in view.", chunkBackgroundEntityId);
		return {};
	}

	const worker::Entity &chunkBackgroundEntity = chunkBackgroundEntityQuery->second;
	worker::Option<const TilemapTilesData &> tilemapTilesComponent = chunkBackgroundEntity.Get<TilemapTiles>();
	if(!tilemapTilesComponent) {
		shovelerLogWarning("Supposed chunk background entity %lld doesn't have a tilemap tiles component.", chunkBackgroundEntityId);
		return {};
	}

	return {{*tilemapTilesComponent->tileset_columns(), *tilemapTilesComponent->tileset_rows(), *tilemapTilesComponent->tileset_ids()}};
}

static Coordinates getNewPlayerPosition(worker::Connection &connection, worker::View &view, const CreateClientEntityRequest& request) {
	const Option<std::string> &flagOption = connection.GetWorkerFlag("game_type");
	if(!(flagOption && *flagOption == "tiles")) {
		return {0, 5, 0};
	}

	for(int i = 0; i < numPlayerPositionAttempts; i++) {
		int minX = 9;
		int minZ = 9;
		int sizeX = 2;
		int sizeZ = 2;
		if(request.starting_chunk_region()) {
			minX = request.starting_chunk_region()->min_x();
			minZ = request.starting_chunk_region()->min_z();
			sizeX = request.starting_chunk_region()->size_x();
			sizeZ = request.starting_chunk_region()->size_z();
			shovelerLogInfo("Overriding starting chunk region to min (%d, %d) and size (%d, %d).", minX, minZ, sizeX, sizeZ);
		}

		int startingChunkX = minX + (rand() % sizeX);
		int startingChunkZ = minZ + (rand() % sizeZ);

		EntityId backgroundChunkEntityId = getChunkBackgroundEntityId(startingChunkX, startingChunkZ);
		Option<TilesData> tiles = getChunkBackgroundTiles(connection, view, backgroundChunkEntityId);
		if(!tiles) {
			continue;
		}

		int startingTileX = rand() % 10;
		int startingTileZ = rand() % 10;
		unsigned char tilesetColumn = tiles->tilesetColumns[startingTileZ * chunkSize + startingTileX];
		unsigned char tilesetRows = tiles->tilesetRows[startingTileZ * chunkSize + startingTileX];
		unsigned char tilesetIds = tiles->tilesetIds[startingTileZ * chunkSize + startingTileX];
		if(tilesetColumn > 2) { // not grass
			continue;
		}

		ShovelerVector2 worldPosition2 = tileToWorld(startingChunkX, startingChunkZ, startingTileX, startingTileZ);
		shovelerLogInfo("Rolled new player position in tile (%d, %d) of chunk (%d, %d) after %d iterations: (%.2f, %.2f)", startingTileX, startingTileZ, startingChunkX, startingChunkZ, i + 1, worldPosition2.values[0], worldPosition2.values[1]);

		int chunkX, chunkZ, tileX, tileZ;
		worldToTile(worldPosition2.values[0] + 0.5, worldPosition2.values[1] + 0.5, chunkX, chunkZ, tileX, tileZ);
		shovelerLogInfo("Back translation: tile (%d, %d) chunk (%d, %d)", tileX, tileZ, chunkX, chunkZ);

		return {worldPosition2.values[0] + 0.5, 5, worldPosition2.values[1] + 0.5};
	}

	shovelerLogInfo("Using default position after %d failed attempts to roll new player position.", numPlayerPositionAttempts);
	return {0.5, 5, 0.5};
}

static WorkerRequirementSet getSpecificWorkerRequirementSet(worker::List<std::string> attributeSet) {
	for(worker::List<std::string>::const_iterator iter = attributeSet.begin(); iter != attributeSet.end(); ++iter) {
		if(g_str_has_prefix(iter->c_str(), "workerId:")) {
			return {{{{*iter}}}};
		}
	}

	return {{{{"workerId:unknown"}}}};
}

static ShovelerVector3 remapImprobablePosition(const Coordinates& coordinates, bool isTiles)
{
	if(isTiles) {
		return shovelerVector3((float) coordinates.x(), (float) coordinates.z(), (float) coordinates.y());
	} else {
		return shovelerVector3((float) -coordinates.x(), (float) coordinates.y(), (float) coordinates.z());
	}
}

static Coordinates remapPosition(const ShovelerVector3& coordinates, bool isTiles)
{
	if(isTiles) {
		return {coordinates.values[0], coordinates.values[2], coordinates.values[1]};
	} else {
		return {-coordinates.values[0], coordinates.values[1], coordinates.values[2]};
	}
}
