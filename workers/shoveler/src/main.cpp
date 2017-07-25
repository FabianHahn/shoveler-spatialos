#include <improbable/worker.h>

int main(int argc, char** argv) {
  if (argc != 4) {
    return 1;
  }

  worker::ConnectionParameters parameters;
  parameters.WorkerType = "ShovelerClient";
  parameters.Network.ConnectionType = worker::NetworkConnectionType::kTcp;
  parameters.Network.UseExternalIp = false;

  const std::string workerId = argv[1];
  const std::string hostname = argv[2];
  const std::uint16_t port = static_cast<std::uint16_t>(std::stoi(argv[3]));

  worker::Connection connection = worker::Connection::ConnectAsync(hostname, port, workerId, parameters).Get();
}
