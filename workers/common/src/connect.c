#include "shoveler/connect.h"

#include <stdlib.h> // atoi rand
#include <string.h> // strlen

#include <glib.h>

#include <improbable/c_worker.h>
#include <shoveler/log.h>

uint8_t locatorQueueStatusCallback(void *unused, const Worker_QueueStatus *queueStatus);

Worker_Connection *shovelerWorkerConnect(int argc, char **argv, int argumentOffset, Worker_ConnectionParameters *connectionParameters)
{
	const char *launcherPrefix = "spatialos.launch:";
	if(argc - argumentOffset == 5) {
		const char *locatorHostname = argv[argumentOffset + 1];
		const char *projectName = argv[argumentOffset + 2];
		const char *deploymentName = argv[argumentOffset + 3];
		const char *loginToken = argv[argumentOffset + 4];

		shovelerLogInfo("Connecting to cloud deployment...\n\tLocator hostname: %s\n\tProject name: %s\n\tDeployment name: %s\n\tLogin token: %s", locatorHostname, projectName, deploymentName, loginToken);

		Worker_LocatorParameters locatorParameters;
		locatorParameters.project_name = projectName;
		locatorParameters.credentials_type = WORKER_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
		locatorParameters.login_token.token = loginToken;

		Worker_Locator *locator = Worker_Locator_Create(locatorHostname, /* port */ 0, &locatorParameters);

		connectionParameters->network.use_external_ip = true;
		Worker_ConnectionFuture *connectionFuture = Worker_Locator_ConnectAndQueueAsync(locator, deploymentName, connectionParameters, NULL, locatorQueueStatusCallback);
		Worker_Connection *connection = Worker_ConnectionFuture_Get(connectionFuture, /* timeoutMillis */ NULL);

		Worker_ConnectionFuture_Destroy(connectionFuture);
		Worker_Locator_Destroy(locator);

		return connection;
	} else if(argc - argumentOffset == 2 && g_str_has_prefix(argv[argumentOffset + 1], launcherPrefix)) {
		const char *launcherString = argv[argumentOffset + 1] + strlen(launcherPrefix);
		gchar **projectNameSplit = g_strsplit(launcherString, "-", 2);
		if(projectNameSplit[0] == NULL || projectNameSplit[1] == NULL) {
			shovelerLogError("Failed to extract project name from launcher string: %s", launcherString);
			return NULL;
		}
		const char *projectName = projectNameSplit[0];
		const char *afterProjectName = projectNameSplit[1];

		gchar **deploymentNameSplit = g_strsplit(afterProjectName, "?", 2);
		if(deploymentNameSplit[0] == NULL || deploymentNameSplit[1] == NULL) {
			g_strfreev(projectNameSplit);
			shovelerLogError("Failed to extract deployment name from launcher string: %s", afterProjectName);
			return NULL;
		}
		const char *deploymentName = deploymentNameSplit[0];
		const char *afterDeploymentName = deploymentNameSplit[1];

		gchar **afterDeploymentNameSplit = g_strsplit(afterDeploymentName, "&", 0);
		GString *authToken = g_string_new("");
		const char *locatorHost = "locator.improbable.io";
		for(int i = 0; afterDeploymentNameSplit[i] != NULL; i++) {
			const char *tokenPrefix = "token=";
			const char *environmentPrefix = "environment=";

			if(g_str_has_prefix(afterDeploymentNameSplit[i], tokenPrefix)) {
				g_string_append(authToken, afterDeploymentNameSplit[i] + strlen(tokenPrefix));
			} else if (g_str_has_prefix(afterDeploymentNameSplit[i], environmentPrefix)) {
				if (strcmp(afterDeploymentNameSplit[i] + strlen(environmentPrefix), "staging") == 0) {
					locatorHost = "locator-staging.improbable.io";
				} else {
					shovelerLogWarning("Launcher URL specifies unsupported environment '%s', connection might fail.", afterDeploymentNameSplit[i] + strlen(environmentPrefix));
				}
			}
		}

		if(authToken->len == 0) {
			shovelerLogError("Failed to extract auth token from launcher string: %s", afterDeploymentName);
			g_strfreev(projectNameSplit);
			g_strfreev(deploymentNameSplit);
			g_strfreev(afterDeploymentNameSplit);
			return NULL;
		}

		shovelerLogInfo("Connecting to cloud deployment...\n\tLocator host: %s\n\tProject name: %s\n\tDeployment name: %s\n\tAuth token: %s", locatorHost, projectName, deploymentName, authToken->str);

		Worker_LocatorParameters locatorParameters;
		memset(&locatorParameters, 0, sizeof(Worker_LocatorParameters));
		locatorParameters.project_name = projectName;
		locatorParameters.credentials_type = WORKER_LOCATOR_LOGIN_TOKEN_CREDENTIALS;
		locatorParameters.login_token.token = authToken->str;

		Worker_Locator *locator = Worker_Locator_Create(locatorHost, /* port */ 0, &locatorParameters);

		connectionParameters->network.use_external_ip = true;
		Worker_ConnectionFuture *connectionFuture = Worker_Locator_ConnectAndQueueAsync(locator, deploymentName, connectionParameters, NULL, locatorQueueStatusCallback);
		Worker_Connection *connection = Worker_ConnectionFuture_Get(connectionFuture, /* timeoutMillis */ NULL);

		Worker_ConnectionFuture_Destroy(connectionFuture);
		Worker_Locator_Destroy(locator);
		g_string_free(authToken, true);

		g_strfreev(projectNameSplit);
		g_strfreev(deploymentNameSplit);
		g_strfreev(afterDeploymentNameSplit);

		return connection;
	} else {
		GString *randomWorkerId = g_string_new(connectionParameters->worker_type);
		g_string_append_printf(randomWorkerId, "Local-%08x", rand());
		const char *workerId = randomWorkerId->str;

		const char *hostname = "localhost";
		uint16_t port = 7777;

		if(argc - argumentOffset == 4) {
			workerId = argv[argumentOffset + 1];
			hostname = argv[argumentOffset + 2];
			port = (uint16_t) atoi(argv[argumentOffset + 3]);
		} else if(argc - argumentOffset != 1) {
			shovelerLogError("Usage:\n\t%s\n\t%s <launcher link>\n\t%s <worker ID> <hostname> <port>", argv[argumentOffset + 0], argv[argumentOffset + 0], argv[argumentOffset + 0]);
			g_string_free(randomWorkerId, true);
			return NULL;
		}

		shovelerLogInfo("Connecting to local deployment...\n\tWorker ID: %s\n\tAddress: %s:%d", workerId, hostname, port);

		Worker_ConnectionFuture *connectionFuture = Worker_ConnectAsync(hostname, port, workerId, connectionParameters);
		Worker_Connection *connection = Worker_ConnectionFuture_Get(connectionFuture, /* timeoutMillis */ NULL);

		Worker_ConnectionFuture_Destroy(connectionFuture);
		g_string_free(randomWorkerId, true);

		return connection;
	}
}

uint8_t locatorQueueStatusCallback(void *unused, const Worker_QueueStatus *queueStatus)
{
	if (queueStatus->error != NULL) {
		shovelerLogError("Error while queueing: %s", queueStatus->error);
		return false;
	}
	shovelerLogInfo("Current position in login queue: %u", queueStatus->position_in_queue);
	return true;
}
