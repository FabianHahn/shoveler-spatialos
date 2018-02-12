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
using improbable::Position;

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
		improbable::Position>{};

	worker::Connection connection = worker::Connection::ConnectAsync(components, hostname, port, workerId, parameters).Get();
	bool disconnected = false;
	worker::Dispatcher dispatcher{components};

	while(!disconnected) {
		dispatcher.Process(connection.GetOpList(1000));
	}
}
