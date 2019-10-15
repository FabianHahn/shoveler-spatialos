#include <cstdlib> // rand

#include "connect.h"

extern "C" {
#include <glib.h>

#include <shoveler/log.h>
}

worker::Option<worker::Connection> connect(int argc, char **argv, worker::ConnectionParameters connectionParameters, const worker::ComponentRegistry& components)
{
	std::string launcherPrefix = "spatialos.launch:";
	if(argc == 5) {
		std::string locatorHostname{argv[1]};
		std::string projectName{argv[2]};
		std::string deploymentName{argv[3]};
		std::string loginToken{argv[4]};

		shovelerLogInfo("Connecting to cloud deployment...\n\tLocator hostname: %s\n\tProject name: %s\n\tDeployment name: %s\n\tLogin token: %s", locatorHostname.c_str(), projectName.c_str(), deploymentName.c_str(), loginToken.c_str());

		worker::LocatorParameters locatorParameters;
		locatorParameters.ProjectName = projectName;
		locatorParameters.CredentialsType = worker::LocatorCredentialsType::kLoginToken;
		locatorParameters.LoginToken = worker::LoginTokenCredentials{loginToken};
		worker::Locator locator{locatorHostname, locatorParameters};

		auto queueStatusCallback = [&](const worker::QueueStatus& queueStatus) {
			if (!queueStatus.Error.empty()) {
				shovelerLogError("Error while queueing: %s", queueStatus.Error->c_str());
				return false;
			}
			shovelerLogInfo("Current position in login queue: %d", queueStatus.PositionInQueue);
			return true;
		};

		connectionParameters.Network.UseExternalIp = true;
		return locator.ConnectAsync(components, deploymentName, connectionParameters, queueStatusCallback).Get();
	} else if(argc == 2 && g_str_has_prefix(argv[1], launcherPrefix.c_str())) {
		const char *launcherString = argv[1] + launcherPrefix.size();
		gchar **projectNameSplit = g_strsplit(launcherString, "-", 2);
		if(projectNameSplit[0] == NULL || projectNameSplit[1] == NULL) {
			shovelerLogError("Failed to extract project name from launcher string: %s", launcherString);
			return {};
		}
		std::string projectName{projectNameSplit[0]};
		std::string afterProjectName{projectNameSplit[1]};
		g_strfreev(projectNameSplit);

		gchar **deploymentNameSplit = g_strsplit(afterProjectName.c_str(), "?", 2);
		if(deploymentNameSplit[0] == NULL || deploymentNameSplit[1] == NULL) {
			shovelerLogError("Failed to extract deployment name from launcher string: %s", afterProjectName.c_str());
			return {};
		}
		std::string deploymentName{deploymentNameSplit[0]};
		std::string afterDeploymentName{deploymentNameSplit[1]};
		g_strfreev(deploymentNameSplit);

		gchar **afterDeploymentNameSplit = g_strsplit(afterDeploymentName.c_str(), "&", 0);
		std::string authToken;
		for(int i = 0; afterDeploymentNameSplit[i] != NULL; i++) {
			std::string tokenPrefix = "token=";
			if(g_str_has_prefix(afterDeploymentNameSplit[i], tokenPrefix.c_str())) {
				authToken = std::string{afterDeploymentNameSplit[i]}.substr(tokenPrefix.size());
				break;
			}
		}
		g_strfreev(afterDeploymentNameSplit);

		if(authToken.empty()) {
			shovelerLogError("Failed to extract auth token from launcher string: %s", afterDeploymentName.c_str());
			return {};
		}

		shovelerLogInfo("Connecting to cloud deployment...\n\tProject name: %s\n\tDeployment name: %s\n\tAuth token: %s", projectName.c_str(), deploymentName.c_str(), authToken.c_str());

		worker::LocatorParameters locatorParameters;
		locatorParameters.ProjectName = projectName;
		locatorParameters.CredentialsType = worker::LocatorCredentialsType::kLoginToken;
		locatorParameters.LoginToken = worker::LoginTokenCredentials{authToken};
		worker::Locator locator{"locator.improbable.io", locatorParameters};

		auto queueStatusCallback = [&](const worker::QueueStatus& queueStatus) {
			if (!queueStatus.Error.empty()) {
				shovelerLogError("Error while queueing: %s", queueStatus.Error->c_str());
				return false;
			}
			shovelerLogInfo("Current position in login queue: %d", queueStatus.PositionInQueue);
			return true;
		};

		connectionParameters.Network.UseExternalIp = true;
		return locator.ConnectAsync(components, deploymentName, connectionParameters, queueStatusCallback).Get();
	} else {
		GString *randomWorkerId = g_string_new(connectionParameters.WorkerType.c_str());
		g_string_append_printf(randomWorkerId, "Local-%08x", rand());
		std::string workerId{randomWorkerId->str, randomWorkerId->len};
		g_string_free(randomWorkerId, true);

		std::string hostname = "localhost";
		std::uint16_t port = 7777;

		if(argc == 4) {
			workerId = argv[1];
			hostname = argv[2];
			port = static_cast<std::uint16_t>(std::stoi(argv[3]));
		} else if(argc != 1) {
			shovelerLogError("Usage:\n\t%s\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>", argv[0], argv[0], argv[0]);
			return {};
		}

		shovelerLogInfo("Connecting to local deployment...\n\tWorker ID: %s\n\tAddress: %s:%d", workerId.c_str(), hostname.c_str(), (unsigned int) port);
		return worker::Connection::ConnectAsync(components, hostname, port, workerId, connectionParameters).Get();
	}
}
