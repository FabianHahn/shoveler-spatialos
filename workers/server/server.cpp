#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>

#include <improbable/standard_library.h>
#include <improbable/view.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <glib.h>

#include <shoveler/color.h>
#include <shoveler/executor.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
}

using improbable::ComponentInterest;
using improbable::EntityAcl;
using improbable::EntityAclData;
using improbable::Interest;
using improbable::Metadata;
using improbable::Persistence;
using improbable::Position;
using improbable::PositionData;
using improbable::WorkerAttributeSet;
using improbable::WorkerRequirementSet;
using shoveler::Bootstrap;
using shoveler::Canvas;
using shoveler::Client;
using shoveler::ClientHeartbeat;
using shoveler::ClientInfo;
using shoveler::ClientInfoData;
using shoveler::Color;
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
using shoveler::Resource;
using shoveler::ResourceData;
using shoveler::TilemapTiles;
using shoveler::TilemapTilesData;
using shoveler::TilemapTilesTile;
using shoveler::TileSprite;
using shoveler::TileSpriteAnimation;
using worker::EntityId;
using worker::List;
using worker::Option;

using CreateClientEntity = shoveler::Bootstrap::Commands::CreateClientEntity;
using ClientPing = shoveler::Bootstrap::Commands::ClientPing;
using ClientSpawnCube = shoveler::Bootstrap::Commands::ClientSpawnCube;
using DigHole = shoveler::Bootstrap::Commands::DigHole;
using UpdateResource = shoveler::Bootstrap::Commands::UpdateResource;
using Query = ComponentInterest::Query;
using QueryConstraint = ComponentInterest::QueryConstraint;

struct ClientCleanupTickContext {
	worker::Connection *connection;
	int64_t maxHeartbeatTimeoutMs;
	std::map<worker::EntityId, std::pair<std::string, int64_t>> *lastHeartbeatMicrosMap;
};

static void clientCleanupTick(void *clientCleanupTickContextPointer);
static Color colorFromHsv(float h, float s, float v);
static WorkerRequirementSet getSpecificWorkerRequirementSet(worker::List<std::string> attributeSet);

int main(int argc, char **argv) {
	if (argc != 5) {
		return 1;
	}

	srand(0);

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "ShovelerServer";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
	parameters.Network.UseExternalIp = false;

	const std::string workerId = argv[1];
	const std::string hostname = argv[2];
	const std::uint16_t port = static_cast<std::uint16_t>(std::stoi(argv[3]));
	const std::string logFileLocation = argv[4];
	const int tickRateHz = 10;
	const int clientCleanupTickRateHz = 2;
	const int64_t maxHeartbeatTimeoutMs = 5000;
	EntityId cubeDrawableEntityId = 2;
	EntityId pointDrawableEntityId = 4;
	const EntityId characterAnimationTilesetEntityId = 5;
	const EntityId character2AnimationTilesetEntityId = 6;
	const EntityId character3AnimationTilesetEntityId = 7;
	const EntityId character4AnimationTilesetEntityId = 8;
	const EntityId canvasEntityId = 9;
	const EntityId firstChunkEntityId = 10;
	const int halfMapWidth = 100;
	const int halfMapHeight = 100;
	const int chunkSize = 10;
	const int numChunkColumns = 2 * halfMapWidth / chunkSize;
	const int numChunkRows = 2 * halfMapHeight / chunkSize;
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
		ClientHeartbeat,
		ClientInfo,
		Drawable,
		EntityAcl,
		Interest,
		Light,
		Material,
		Metadata,
		Model,
		Persistence,
		Position,
		Resource,
		TilemapTiles,
		TileSprite,
		TileSpriteAnimation>{};

	worker::Connection connection = worker::Connection::ConnectAsync(components, hostname, port, workerId, parameters).Get();
	bool disconnected = false;
	worker::View view{components};
	worker::Dispatcher& dispatcher = view;

	ShovelerExecutor *tickExecutor = shovelerExecutorCreateDirect();
	std::map<worker::EntityId, std::pair<std::string, int64_t>> lastHeartbeatMicrosMap;

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		shovelerLogError("Disconnected from SpatialOS: %s", op.Reason.c_str());
		disconnected = true;
	});

	dispatcher.OnMetrics([&](const worker::MetricsOp& op) {
		auto metrics = op.Metrics;
		connection.SendMetrics(metrics);
	});

	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		switch(op.Level) {
			case worker::LogLevel::kDebug:
				shovelerLogTrace(op.Message.c_str());
				break;
			case worker::LogLevel::kInfo:
				shovelerLogInfo(op.Message.c_str());
				break;
			case worker::LogLevel::kWarn:
				shovelerLogWarning(op.Message.c_str());
				break;
			case worker::LogLevel::kError:
				shovelerLogError(op.Message.c_str());
				break;
			case worker::LogLevel::kFatal:
				shovelerLogError(op.Message.c_str());
				std::terminate();
			default:
				break;
		}
	});

	dispatcher.OnAddEntity([&](const worker::AddEntityOp& op) {
		shovelerLogInfo("Adding entity %lld.", op.EntityId);
	});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op) {
		shovelerLogInfo("Removing entity %lld.", op.EntityId);
	});

	dispatcher.OnAddComponent<Bootstrap>([&](const worker::AddComponentOp<Bootstrap>& op) {
		shovelerLogInfo("Adding bootstrap to entity %lld.", op.EntityId);
	});

	dispatcher.OnComponentUpdate<Bootstrap>([&](const worker::ComponentUpdateOp<Bootstrap>& op) {
		shovelerLogInfo("Updating bootstrap for entity %lld.", op.EntityId);
	});

	dispatcher.OnRemoveComponent<Bootstrap>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removing bootstrap from entity %lld.", op.EntityId);
	});

	dispatcher.OnAuthorityChange<Bootstrap>([&](const worker::AuthorityChangeOp& op) {
		shovelerLogInfo("Authority change to %d for entity %lld.", op.Authority, op.EntityId);
	});

	dispatcher.OnAddComponent<ClientHeartbeat>([&](const worker::AddComponentOp<ClientHeartbeat>& op) {
		lastHeartbeatMicrosMap[op.EntityId] = std::make_pair("(unknown)", view.Entities[op.EntityId].Get<ClientHeartbeat>()->last_server_heartbeat());
		shovelerLogInfo("Added client heartbeat for entity %lld.", op.EntityId);
	});

	dispatcher.OnComponentUpdate<ClientHeartbeat>([&](const worker::ComponentUpdateOp<ClientHeartbeat>& op) {
		if (lastHeartbeatMicrosMap.find(op.EntityId) != lastHeartbeatMicrosMap.end()) {
			std::string clientWorkerId = "(unknown)";
			const auto &clientComponentOption = view.Entities[op.EntityId].Get<Client>();
			if (clientComponentOption) {
				clientWorkerId = clientComponentOption->worker_id();
			}

			lastHeartbeatMicrosMap[op.EntityId] = std::make_pair(clientWorkerId, view.Entities[op.EntityId].Get<ClientHeartbeat>()->last_server_heartbeat());
		}
		shovelerLogTrace("Updated client heartbeat for entity %lld.", op.EntityId);
	});

	dispatcher.OnRemoveComponent<ClientHeartbeat>([&](const worker::RemoveComponentOp& op) {
		shovelerLogInfo("Removed client heartbeat for entity %lld.", op.EntityId);
	});

	dispatcher.OnAuthorityChange<ClientHeartbeat>([&](const worker::AuthorityChangeOp& op) {
		shovelerLogInfo("Changing heartbeat authority for entity %lld to %d.", op.EntityId, op.Authority);
		if (op.Authority == worker::Authority::kAuthoritative) {
			int64_t initialHeartbeat = g_get_monotonic_time() + 5 * 1000 * maxHeartbeatTimeoutMs;
			lastHeartbeatMicrosMap[op.EntityId] = std::make_pair("(unknown)", initialHeartbeat);

			ClientHeartbeat::Update heartbeatUpdate;
			heartbeatUpdate.set_last_server_heartbeat(initialHeartbeat);
			connection.SendComponentUpdate<ClientHeartbeat>(op.EntityId, heartbeatUpdate);
		} else if (op.Authority == worker::Authority::kNotAuthoritative) {
			lastHeartbeatMicrosMap.erase(op.EntityId);
		}
	});

	dispatcher.OnCommandRequest<CreateClientEntity>([&](const worker::CommandRequestOp<CreateClientEntity>& op) {
		shovelerLogInfo("Received create client entity request from: %s", op.CallerWorkerId.c_str());

		worker::Entity clientEntity;
		clientEntity.Add<Metadata>({"client"});
		clientEntity.Add<Client>({op.CallerWorkerId});
		clientEntity.Add<ClientHeartbeat>({0});
		clientEntity.Add<Persistence>({});
		clientEntity.Add<Position>({{0, 0, 0}});

		QueryConstraint relativeConstraint;
		relativeConstraint.set_relative_box_constraint({{{20, 9999, 20}}});
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
		clientEntityComponentAclMap.insert({{ClientHeartbeat::ComponentId, serverRequirementSet}});
		clientEntityComponentAclMap.insert({{Interest::ComponentId, specificClientRequirementSet}});
		clientEntityComponentAclMap.insert({{Position::ComponentId, specificClientRequirementSet}});
		EntityAclData clientEntityAclData(clientAndServerRequirementSet, clientEntityComponentAclMap);
		clientEntity.Add<EntityAcl>(clientEntityAclData);

		const Option<std::string> &flagOption = connection.GetWorkerFlag("game_type");
		if (flagOption && *flagOption == "tiles") {
			int modulo = characterCounter++ % 4;
			worker::EntityId tilesetEntityId = characterAnimationTilesetEntityId;
			if(modulo == 1) {
				tilesetEntityId = character2AnimationTilesetEntityId;
			} else if(modulo == 2) {
				tilesetEntityId = character3AnimationTilesetEntityId;
			} else if(modulo == 3) {
				tilesetEntityId = character4AnimationTilesetEntityId;
			}

			clientEntity.Add<TileSprite>({tilesetEntityId, 0, 0, CoordinateMapping::POSITIVE_X, CoordinateMapping::POSITIVE_Y, {1.0f, 1.0f}});
			clientEntity.Add<TileSpriteAnimation>({0, 0.5});
		} else {
			float hue = (float) rand() / RAND_MAX;
			float saturation = 0.5f + 0.5f * ((float) rand() / RAND_MAX);
			Color playerParticleColor = colorFromHsv(hue, saturation, 0.9f);
			Color playerLightColor = colorFromHsv(hue, saturation, 0.1f);

			clientEntity.Add<ClientInfo>({hue, saturation});
			clientEntity.Add<Material>({MaterialType::PARTICLE, playerParticleColor, {}, {}, {}});
			clientEntity.Add<Model>({pointDrawableEntityId, 0, {0.0f, 0.0f, 0.0f}, {0.1f, 0.1f, 0.0f}, true, true, false, false, PolygonMode::FILL});
			clientEntity.Add<Light>({LightType::POINT, 1024, 1024, 1, 0.01f, 80.0f, playerLightColor, {}});
		}

		worker::RequestId<worker::CreateEntityRequest> createEntityRequestId = connection.SendCreateEntityRequest(clientEntity, {}, {});
		shovelerLogInfo("Sent create entity request %u.", createEntityRequestId.Id);

		connection.SendCommandResponse<CreateClientEntity>(op.RequestId, {});
	});

	dispatcher.OnCommandRequest<ClientPing>([&](const worker::CommandRequestOp<ClientPing>& op) {
		worker::EntityId clientEntityId = op.Request.client_entity_id();
		worker::Map<worker::EntityId, worker::Map<worker::ComponentId, worker::Authority>>::const_iterator entityAuthorityQuery = view.ComponentAuthority.find(clientEntityId);
		if(entityAuthorityQuery == view.ComponentAuthority.end()) {
			shovelerLogWarning("Received client ping from %s for unknown client entity %lld, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}
		const worker::Map<worker::ComponentId, worker::Authority>& componentAuthorityMap = entityAuthorityQuery->second;

		worker::Map<worker::ComponentId, worker::Authority>::const_iterator componentAuthorityQuery = componentAuthorityMap.find(ClientHeartbeat::ComponentId);
		if(componentAuthorityQuery == componentAuthorityMap.end()) {
			shovelerLogWarning("Received client ping from %s for client entity %lld without client heartbeat component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}
		const worker::Authority& HeartbeatAuthority = componentAuthorityQuery->second;

		if(HeartbeatAuthority != worker::Authority::kAuthoritative) {
			shovelerLogWarning("Received client ping from %s for client entity %lld without authoritative client heartbeat component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}

		int64_t now = g_get_monotonic_time();

		ClientHeartbeat::Update heartbeatUpdate;
		heartbeatUpdate.set_last_server_heartbeat(now);
		connection.SendComponentUpdate<ClientHeartbeat>(clientEntityId, heartbeatUpdate);
		connection.SendCommandResponse<ClientPing>(op.RequestId, {now});

		shovelerLogTrace("Received client ping from %s for client entity %lld.", op.CallerWorkerId.c_str(), clientEntityId);
	});

	dispatcher.OnCommandRequest<ClientSpawnCube>([&](const worker::CommandRequestOp<ClientSpawnCube>& op) {
		worker::EntityId clientEntityId = op.Request.client_entity_id();
		shovelerLogInfo("Received client spawn cube from %s for client entity %lld in direction (%.2f, %.2f, %.2f).", op.CallerWorkerId.c_str(), clientEntityId, op.Request.direction().x(), op.Request.direction().y(), op.Request.direction().z());

		worker::Map<worker::EntityId, worker::Entity>::const_iterator entityQuery = view.Entities.find(clientEntityId);
		if(entityQuery == view.Entities.end()) {
			shovelerLogWarning("Received client spawn cube from %s for unknown client entity %lld, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}

		const worker::Entity& clientEntity = entityQuery->second;
		worker::Option<const ClientInfoData&> clientInfoComponent = clientEntity.Get<ClientInfo>();
		if(!clientInfoComponent) {
			shovelerLogWarning("Received client spawn cube from %s for client entity %lld without client info component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}
		worker::Option<const PositionData&> positionComponent = clientEntity.Get<Position>();
		if(!positionComponent) {
			shovelerLogWarning("Received client spawn cube from %s for client entity %lld without position component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}

		ShovelerVector3 normalizedDirection = shovelerVector3Normalize(shovelerVector3(op.Request.direction().x(), op.Request.direction().y(), op.Request.direction().z()));
		ShovelerVector3 cubePosition = shovelerVector3LinearCombination(1.0f, shovelerVector3(positionComponent->coords().x(), positionComponent->coords().y(), positionComponent->coords().z()), 0.5f, normalizedDirection);

		Color cubeColor = colorFromHsv(clientInfoComponent->color_hue(), clientInfoComponent->color_saturation(), 0.7f);

		worker::Entity cubeEntity;
		cubeEntity.Add<Metadata>({"cube"});
		cubeEntity.Add<Persistence>({});
		cubeEntity.Add<Position>({{cubePosition.values[0], cubePosition.values[1], cubePosition.values[2]}});
		cubeEntity.Add<Material>({MaterialType::COLOR, cubeColor, {}, {}, {}});
		cubeEntity.Add<Model>({cubeDrawableEntityId, 0, op.Request.rotation(), {0.25f, 0.25f, 0.25f}, true, false, false, true, PolygonMode::FILL});

		WorkerAttributeSet clientAttributeSet({"client"});
		WorkerRequirementSet clientRequirementSet({clientAttributeSet});
		cubeEntity.Add<EntityAcl>({clientRequirementSet, {}});

		connection.SendCreateEntityRequest(cubeEntity, {}, {});
		connection.SendCommandResponse<ClientSpawnCube>(op.RequestId, {});
	});

	dispatcher.OnCommandRequest<DigHole>([&](const worker::CommandRequestOp<DigHole>& op) {
		worker::EntityId clientEntityId = op.Request.client_entity_id();
		shovelerLogInfo("Received dig hole request from %s for client entity %lld.", op.CallerWorkerId.c_str(), clientEntityId);

		worker::Map<worker::EntityId, worker::Entity>::const_iterator entityQuery = view.Entities.find(clientEntityId);
		if(entityQuery == view.Entities.end()) {
			shovelerLogWarning("Received dig hole request from %s for unknown client entity %lld, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "unknown client entity");
			return;
		}

		const worker::Entity& clientEntity = entityQuery->second;
		worker::Option<const PositionData&> positionComponent = clientEntity.Get<Position>();
		if(!positionComponent) {
			shovelerLogWarning("Received dig hole request from %s for client entity %lld without position component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "client entity without position");
			return;
		}

		double x = positionComponent->coords().x();
		double z = positionComponent->coords().z();

		double diffX = x + halfMapWidth;
		double diffZ = z + halfMapHeight;

		int chunkX = (int) floor(diffX / chunkSize);
		int chunkZ = (int) floor(diffZ / chunkSize);

		int tileX = (int) floor(diffX - chunkX * chunkSize);
		int tileZ = (int) floor(diffZ - chunkZ * chunkSize);

		if(chunkX < 0 || chunkX >= numChunkColumns || chunkZ < 0 || chunkZ >= numChunkRows || tileX < 0 || tileX >= chunkSize || tileZ < 0 || tileZ >= chunkSize) {
			shovelerLogWarning("Received dig hole request from %s for client entity %lld which is out of range at (%f, %f), ignoring.", op.CallerWorkerId.c_str(), clientEntityId, x, z);
			connection.SendCommandFailure<DigHole>(op.RequestId, "out of range");
			return;
		}

		worker::EntityId chunkBackgroundEntityId = firstChunkEntityId + 3 * chunkX * numChunkColumns + 3 * chunkZ;
		worker::Map<worker::EntityId, worker::Entity>::const_iterator chunkBackgroundEntityQuery = view.Entities.find(chunkBackgroundEntityId);
		if(chunkBackgroundEntityQuery == view.Entities.end()) {
			shovelerLogError("Received dig hole request from %s for client entity %lld, but chunk background entity %lld is not in view.", op.CallerWorkerId.c_str(), clientEntityId, chunkBackgroundEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "tiles not in view");
			return;
		}

		const worker::Entity& chunkBackgroundEntity = chunkBackgroundEntityQuery->second;
		worker::Option<const TilemapTilesData&> tilemapTilesComponent = chunkBackgroundEntity.Get<TilemapTiles>();
		if(!tilemapTilesComponent) {
			shovelerLogError("Received dig hole request from %s for client entity %lld, but chunk background entity %lld doesn't have a tilemap tiles component.", op.CallerWorkerId.c_str(), clientEntityId, chunkBackgroundEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "tiles entity doesn't have tiles component");
			return;
		}

		List<TilemapTilesTile> tiles = tilemapTilesComponent->tiles();
		TilemapTilesTile& tile = tiles[tileZ * chunkSize + tileX];

		if(tile.tileset_column() > 2) {
			shovelerLogWarning("Received dig hole request from %s for client entity %lld, but its current tile is not grass.", op.CallerWorkerId.c_str(), clientEntityId);
			connection.SendCommandFailure<DigHole>(op.RequestId, "tile isn't grass");
			return;
		}

		tile.set_tileset_column(6);
		tile.set_tileset_row(1);
		tile.set_tileset_id(2);

		TilemapTiles::Update tilemapTilesUpdate;
		tilemapTilesUpdate.set_tiles(tiles);
		connection.SendComponentUpdate<TilemapTiles>(chunkBackgroundEntityId, tilemapTilesUpdate);
		connection.SendCommandResponse<DigHole>(op.RequestId, {});
	});

	dispatcher.OnCommandRequest<UpdateResource>([&](const worker::CommandRequestOp<UpdateResource>& op) {
		worker::EntityId resourceEntityId = op.Request.resource_entity_id();
		shovelerLogInfo("Received update request from %s for resource entity %lld.", op.CallerWorkerId.c_str(), resourceEntityId);

		worker::Map<worker::EntityId, worker::Entity>::const_iterator entityQuery = view.Entities.find(resourceEntityId);
		if(entityQuery == view.Entities.end()) {
			shovelerLogWarning("Received update request from %s for unknown resource entity %lld, ignoring.", op.CallerWorkerId.c_str(), resourceEntityId);
			connection.SendCommandFailure<UpdateResource>(op.RequestId, "unknown resource entity");
			return;
		}

		const worker::Entity& resourceEntity = entityQuery->second;
		worker::Option<const ResourceData&> resourceComponent = resourceEntity.Get<Resource>();
		if(!resourceComponent) {
			shovelerLogWarning("Received update request from %s for resource entity %lld without resource component, ignoring.", op.CallerWorkerId.c_str(), resourceEntityId);
			connection.SendCommandFailure<UpdateResource>(op.RequestId, "client entity without position");
			return;
		}

		Resource::Update resourceUpdate;
		resourceUpdate.set_type_id(op.Request.type_id());
		resourceUpdate.set_content(op.Request.content());
		connection.SendComponentUpdate<Resource>(resourceEntityId, resourceUpdate);
		connection.SendCommandResponse<UpdateResource>(op.RequestId, {});
	});

	dispatcher.OnCreateEntityResponse([&](const worker::CreateEntityResponseOp& op) {
		shovelerLogInfo("Received create entity response for request %u with status code %d: %s", op.RequestId.Id, op.StatusCode, op.Message.c_str());

		if(op.StatusCode == worker::StatusCode::kSuccess) {
			const worker::Map<worker::EntityId, worker::Entity>::iterator &query = view.Entities.find(canvasEntityId);
			if(query != view.Entities.end()) {
				const worker::Entity& entity = query->second;
				const worker::Map<worker::ComponentId, worker::Authority>& entityAuthority = view.ComponentAuthority[canvasEntityId];

				if(entity.Get<Canvas>() && entityAuthority.find(Canvas::ComponentId)->second == worker::Authority::kAuthoritative) {
					worker::List<worker::EntityId> tileSpriteEntityIds = entity.Get<Canvas>()->tile_sprite_entity_ids();
					tileSpriteEntityIds.emplace_back(*op.EntityId);

					Canvas::Update canvasUpdate;
					canvasUpdate.set_tile_sprite_entity_ids(tileSpriteEntityIds);
					connection.SendComponentUpdate<Canvas>(canvasEntityId, canvasUpdate);

					shovelerLogInfo("Sent component update to add new client entity %lld to canvas entity's tile sprites list.", *op.EntityId);
				}
			}
		}
	});

	dispatcher.OnDeleteEntityResponse([&](const worker::DeleteEntityResponseOp& op) {
		shovelerLogInfo("Received delete entity response for request %u with status code %d: %s", op.RequestId.Id, op.StatusCode, op.Message.c_str());

		if(op.StatusCode == worker::StatusCode::kSuccess) {
			const worker::Map<worker::EntityId, worker::Entity>::iterator &query = view.Entities.find(canvasEntityId);
			if(query != view.Entities.end()) {
				const worker::Entity& entity = query->second;
				const worker::Map<worker::ComponentId, worker::Authority>& entityAuthority = view.ComponentAuthority[canvasEntityId];

				if(entity.Get<Canvas>() && entityAuthority.find(Canvas::ComponentId)->second == worker::Authority::kAuthoritative) {
					worker::List<worker::EntityId> tileSpriteEntityIds = entity.Get<Canvas>()->tile_sprite_entity_ids();
					for(worker::List<worker::EntityId>::iterator iter = tileSpriteEntityIds.begin(); iter != tileSpriteEntityIds.end();) {
						if(*iter == op.EntityId) {
							iter = tileSpriteEntityIds.erase(iter);
						} else {
							++iter;
						}
					}

					Canvas::Update canvasUpdate;
					canvasUpdate.set_tile_sprite_entity_ids(tileSpriteEntityIds);
					connection.SendComponentUpdate<Canvas>(canvasEntityId, canvasUpdate);

					shovelerLogInfo("Sent component update to remove deleted client entity %lld from canvas entity's tile sprites list.", op.EntityId);
				}
			}
		}
	});

	ClientCleanupTickContext cleanupTickContext{&connection, maxHeartbeatTimeoutMs, &lastHeartbeatMicrosMap};
	uint32_t clientCleanupTickPeriod = (uint32_t) (1000.0 / (double) clientCleanupTickRateHz);
	shovelerExecutorSchedulePeriodic(tickExecutor, 0, clientCleanupTickPeriod, clientCleanupTick, &cleanupTickContext);

	uint32_t executorTickPeriod = (uint32_t) (1000.0 / (double) tickRateHz);
	while(!disconnected) {
		dispatcher.Process(connection.GetOpList(executorTickPeriod));
		shovelerExecutorUpdateNow(tickExecutor);
	}

	shovelerExecutorFree(tickExecutor);
}

static void clientCleanupTick(void *clientCleanupTickContextPointer)
{
	ClientCleanupTickContext *context = (ClientCleanupTickContext *) clientCleanupTickContextPointer;

	int64_t now = g_get_monotonic_time();
	for(const auto& item: *context->lastHeartbeatMicrosMap) {
		worker::EntityId entityId = item.first;
		const std::string &workerId = item.second.first;
		int64_t last = item.second.second;
		int64_t diff = now - last;
		if (diff > 1000 * context->maxHeartbeatTimeoutMs) {
			worker::RequestId<worker::DeleteEntityRequest> deleteEntityRequestId = context->connection->SendDeleteEntityRequest(item.first, {});
			shovelerLogWarning("Sent remove client entity %lld request %u of worker %s because it exceeded the maximum heartbeat timeout of %lldms.", entityId, deleteEntityRequestId, workerId.c_str(), context->maxHeartbeatTimeoutMs);
		}
	}
}

static Color colorFromHsv(float h, float s, float v)
{
	ShovelerColor colorRgb = shovelerColorFromHsv(h, s, v);
	ShovelerVector3 colorFloat = shovelerColorToVector3(colorRgb);

	return Color{colorFloat.values[0], colorFloat.values[1], colorFloat.values[2]};
}

static WorkerRequirementSet getSpecificWorkerRequirementSet(worker::List<std::string> attributeSet)
{
	for(worker::List<std::string>::const_iterator iter = attributeSet.begin(); iter != attributeSet.end(); ++iter) {
		if (g_str_has_prefix(iter->c_str(), "workerId:")) {
			return {{{{*iter}}}};
		}
	}

	return {{{{"workerId:unknown"}}}};
}
