#include <cstdio>
#include <cstring>
#include <iostream>

#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <shoveler/spatialos/log.h>
#include <shoveler/spatialos/worker/view/base/view.h>
#include <shoveler/spatialos/worker/view/base/position.h>
}

using shoveler::Bootstrap;
using CreateClientEntity = shoveler::Bootstrap::Commands::CreateClientEntity;
using shoveler::Client;
using shoveler::CreateClientEntityRequest;
using shoveler::CreateClientEntityResponse;
using improbable::EntityAcl;
using improbable::EntityAclData;
using improbable::Metadata;
using improbable::Persistence;
using improbable::Position;
using improbable::WorkerAttributeSet;
using improbable::WorkerRequirementSet;

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

	const worker::ComponentRegistry& components = worker::Components<
		shoveler::Bootstrap,
		shoveler::Client,
		improbable::EntityAcl,
		improbable::Metadata,
		improbable::Persistence,
		improbable::Position>{};

	worker::Connection connection = worker::Connection::ConnectAsync(components, hostname, port, workerId, parameters).Get();
	bool disconnected = false;
	worker::Dispatcher dispatcher{components};

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
		worker::Entity clientEntity;
		clientEntity.Add<Metadata>({"client"});
		clientEntity.Add<Client>({});
		clientEntity.Add<Persistence>({});
		clientEntity.Add<Position>({{0, 0, 0}});

		WorkerAttributeSet serverAttributeSet({"server"});
		WorkerRequirementSet clientRequirementSet({op.CallerAttributeSet});
		WorkerRequirementSet clientAndServerRequirementSet({op.CallerAttributeSet, serverAttributeSet});
		worker::Map<std::uint32_t, WorkerRequirementSet> clientEntityComponentAclMap;
		clientEntityComponentAclMap.insert({{Client::ComponentId, clientRequirementSet}});
		clientEntityComponentAclMap.insert({{Position::ComponentId, clientRequirementSet}});
		EntityAclData clientEntityAclData(clientAndServerRequirementSet, clientEntityComponentAclMap);
		clientEntity.Add<EntityAcl>(clientEntityAclData);
		connection.SendCreateEntityRequest(clientEntity, {}, {});
		connection.SendCommandResponse<CreateClientEntity>(op.RequestId, {});
	});

	while(!disconnected) {
		dispatcher.Process(connection.GetOpList(1000));
	}
}
