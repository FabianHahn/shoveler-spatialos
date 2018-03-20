#include <cstdio>
#include <cstring>
#include <iostream>

#include <improbable/standard_library.h>
#include <improbable/view.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <glib.h>

#include <shoveler/spatialos/log.h>
#include <shoveler/spatialos/worker/view/base/view.h>
#include <shoveler/spatialos/worker/view/base/position.h>
}

using shoveler::Bootstrap;
using shoveler::Client;
using shoveler::ClientHeartbeat;
using shoveler::Color;
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
using improbable::EntityAcl;
using improbable::EntityAclData;
using improbable::Metadata;
using improbable::Persistence;
using improbable::Position;
using improbable::WorkerAttributeSet;
using improbable::WorkerRequirementSet;

using CreateClientEntity = shoveler::Bootstrap::Commands::CreateClientEntity;
using ClientPing = shoveler::Bootstrap::Commands::ClientPing;

int main(int argc, char **argv) {
	if (argc != 5) {
		return 1;
	}

	worker::ConnectionParameters parameters;
	parameters.WorkerType = "ShovelerServer";
	parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
	parameters.Network.UseExternalIp = false;

	const std::string workerId = argv[1];
	const std::string hostname = argv[2];
	const std::uint16_t port = static_cast<std::uint16_t>(std::stoi(argv[3]));
	const std::string logFileLocation = argv[4];

	FILE *logFile = fopen(logFileLocation.c_str(), "r+");
	if(logFile == NULL) {
		std::cerr << "Failed to open output log file at " << logFileLocation << ": " << strerror(errno) << std::endl;
		return 1;
	}

	shovelerSpatialosLogInit(SHOVELER_SPATIALOS_LOG_LEVEL_ALL, logFile);
	shovelerSpatialosLogInfo("Connecting as worker %s to %s:%d.", workerId.c_str(), hostname.c_str(), port);

	auto components = worker::Components<
		shoveler::Bootstrap,
		shoveler::Client,
		shoveler::ClientHeartbeat,
		shoveler::Light,
		shoveler::Model,
		improbable::EntityAcl,
		improbable::Metadata,
		improbable::Persistence,
		improbable::Position>{};

	worker::Connection connection = worker::Connection::ConnectAsync(components, hostname, port, workerId, parameters).Get();
	bool disconnected = false;
	worker::View view{components};
	worker::Dispatcher& dispatcher = view;

	dispatcher.OnDisconnect([&](const worker::DisconnectOp& op) {
		shovelerSpatialosLogError("Disconnected from SpatialOS: %s", op.Reason.c_str());
		disconnected = true;
	});

	dispatcher.OnMetrics([&](const worker::MetricsOp& op) {
		auto metrics = op.Metrics;
		connection.SendMetrics(metrics);
	});

	dispatcher.OnLogMessage([&](const worker::LogMessageOp& op) {
		switch(op.Level) {
			case worker::LogLevel::kDebug:
				shovelerSpatialosLogTrace(op.Message.c_str());
				break;
			case worker::LogLevel::kInfo:
				shovelerSpatialosLogInfo(op.Message.c_str());
				break;
			case worker::LogLevel::kWarn:
				shovelerSpatialosLogWarning(op.Message.c_str());
				break;
			case worker::LogLevel::kError:
				shovelerSpatialosLogError(op.Message.c_str());
				break;
			case worker::LogLevel::kFatal:
				shovelerSpatialosLogError(op.Message.c_str());
				std::terminate();
			default:
				break;
		}
	});

	dispatcher.OnAddEntity([&](const worker::AddEntityOp& op) {
		shovelerSpatialosLogInfo("Adding entity %lld.", op.EntityId);
	});

	dispatcher.OnRemoveEntity([&](const worker::RemoveEntityOp& op) {
		shovelerSpatialosLogInfo("Removing entity %lld.", op.EntityId);
	});

	dispatcher.OnAddComponent<Bootstrap>([&](const worker::AddComponentOp<Bootstrap>& op) {
		shovelerSpatialosLogInfo("Adding bootstrap to entity %lld.", op.EntityId);
	});

	dispatcher.OnComponentUpdate<Bootstrap>([&](const worker::ComponentUpdateOp<Bootstrap>& op) {
		shovelerSpatialosLogInfo("Updating bootstrap for entity %lld.", op.EntityId);
	});

	dispatcher.OnRemoveComponent<Bootstrap>([&](const worker::RemoveComponentOp& op) {
		shovelerSpatialosLogInfo("Removing bootstrap from entity %lld.", op.EntityId);
	});

	dispatcher.OnAuthorityChange<Bootstrap>([&](const worker::AuthorityChangeOp& op) {
		shovelerSpatialosLogInfo("Authority change to %d for entity %lld.", op.Authority, op.EntityId);
	});

	dispatcher.OnCommandRequest<CreateClientEntity>([&](const worker::CommandRequestOp<CreateClientEntity>& op) {
		shovelerSpatialosLogInfo("Received create client entity request from: %s", op.CallerWorkerId.c_str());

		Color whiteColor{1.0f, 1.0f, 1.0f};

		Drawable pointDrawable{DrawableType::POINT};
		Material whiteParticleMaterial{MaterialType::PARTICLE, whiteColor, {}};

		GDateTime *now = g_date_time_new_now_utc();
		int64_t timestamp = g_date_time_to_unix(now);
		g_date_time_unref(now);

		worker::Entity clientEntity;
		clientEntity.Add<Metadata>({"client"});
		clientEntity.Add<Client>({});
		clientEntity.Add<ClientHeartbeat>({timestamp});
		clientEntity.Add<Persistence>({});
		clientEntity.Add<Position>({{0, 0, 0}});
		clientEntity.Add<Model>({pointDrawable, whiteParticleMaterial, {0.0f, 0.0f, 0.0f}, {0.1f, 0.1f, 0.0f}, true, true, false, false, PolygonMode::FILL});
		clientEntity.Add<Light>({LightType::POINT, 1024, 1024, 1, 0.01f, 80.0f, {0.1f, 0.1f, 0.1f}, {}});

		WorkerAttributeSet clientAttributeSet({"client"});
		WorkerAttributeSet serverAttributeSet({"server"});
		WorkerRequirementSet specificClientRequirementSet({op.CallerAttributeSet});
		WorkerRequirementSet serverRequirementSet({serverAttributeSet});
		WorkerRequirementSet clientAndServerRequirementSet({clientAttributeSet, serverAttributeSet});
		worker::Map<std::uint32_t, WorkerRequirementSet> clientEntityComponentAclMap;
		clientEntityComponentAclMap.insert({{Client::ComponentId, specificClientRequirementSet}});
		clientEntityComponentAclMap.insert({{ClientHeartbeat::ComponentId, serverRequirementSet}});
		clientEntityComponentAclMap.insert({{Position::ComponentId, specificClientRequirementSet}});
		EntityAclData clientEntityAclData(clientAndServerRequirementSet, clientEntityComponentAclMap);
		clientEntity.Add<EntityAcl>(clientEntityAclData);
		connection.SendCreateEntityRequest(clientEntity, {}, {});
		connection.SendCommandResponse<CreateClientEntity>(op.RequestId, {});
	});

	dispatcher.OnCommandRequest<ClientPing>([&](const worker::CommandRequestOp<ClientPing>& op) {
		worker::EntityId clientEntityId = op.Request.client_entity_id();
		worker::Map<worker::EntityId, worker::Map<worker::ComponentId, worker::Authority>>::const_iterator entityAuthorityQuery = view.ComponentAuthority.find(clientEntityId);
		if(entityAuthorityQuery == view.ComponentAuthority.end()) {
			shovelerSpatialosLogWarning("Received client ping from %s for unknown client entity %lld, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}
		const worker::Map<worker::ComponentId, worker::Authority>& componentAuthorityMap = entityAuthorityQuery->second;

		worker::Map<worker::ComponentId, worker::Authority>::const_iterator componentAuthorityQuery = componentAuthorityMap.find(ClientHeartbeat::ComponentId);
		if(componentAuthorityQuery == componentAuthorityMap.end()) {
			shovelerSpatialosLogWarning("Received client ping from %s for client entity %lld without client heartbeat component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}
		const worker::Authority& clientHeartbeatAuthority = componentAuthorityQuery->second;

		if(clientHeartbeatAuthority != worker::Authority::kAuthoritative) {
			shovelerSpatialosLogWarning("Received client ping from %s for client entity %lld without authoritative client heartbeat component, ignoring.", op.CallerWorkerId.c_str(), clientEntityId);
			return;
		}

		ClientHeartbeat::Update clientHeartbeatUpdate;
		clientHeartbeatUpdate.set_last_heartbeat(op.Request.heartbeat());
		connection.SendComponentUpdate<ClientHeartbeat>(clientEntityId, clientHeartbeatUpdate);

		shovelerSpatialosLogWarning("Received client ping from %s for client entity %lld: %lld.", op.CallerWorkerId.c_str(), clientEntityId, op.Request.heartbeat());
	});

	while(!disconnected) {
		dispatcher.Process(connection.GetOpList(1000));
	}
}
