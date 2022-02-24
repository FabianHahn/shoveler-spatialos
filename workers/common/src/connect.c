#include "shoveler/connect.h"

#include <errno.h>
#include <stdlib.h> // atoi rand
#include <string.h> // strlen

#include <glib.h>

#include <improbable/c_worker.h>
#include <shoveler/log.h>

#define MAXIMUM_AUTHORIZATION_TOKEN_SIZE 512

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

static void generateAuthorizationToken(char* output_token, const char* program_path,
	const char* worker_id, const char* worker_type, const char* runtime_id) {
	char program_parent_path[4096];
	char jwtmaker_path[4500];
	char private_key_path[4500];
	
	strcpy(program_parent_path, program_path);
	
	char* program_path_end = strrchr(program_path, PATH_SEPARATOR);
	if(program_path_end == NULL) {
		strcpy(program_parent_path, ".");
	} else {
		program_parent_path[program_path_end - program_path] = '\0';
	}
	snprintf(jwtmaker_path, sizeof(jwtmaker_path), "%s%cjwtmaker", 
		program_parent_path, PATH_SEPARATOR);
	snprintf(private_key_path, sizeof(jwtmaker_path), "%s%cprivate_key.pem", 
		program_parent_path, PATH_SEPARATOR);

	char jwtmaker_command[10000];
	snprintf(jwtmaker_command, sizeof(jwtmaker_command), 
		"%s -worker_id %s -worker_type %s -runtime_id %s -pem_private_key_file %s",
		jwtmaker_path,
		worker_id, worker_type, runtime_id,
		private_key_path);
	
	FILE* pipe = popen(jwtmaker_command, "r");
	if(pipe != NULL) {
		if(fgets(output_token, MAXIMUM_AUTHORIZATION_TOKEN_SIZE, pipe) == NULL) {
			shovelerLogError("Failed to read authorization token: %s\n", strerror(errno));
			exit(1);
		}
		// Remove the extra newline
		int token_size = strlen(output_token);
		if(token_size > 0 && output_token[token_size-1] == '\n') {
			output_token[token_size-1] = '\0';
		}
	}
	else {
		shovelerLogError("Failed to open authorization token helper %s: %s\n", jwtmaker_path, strerror(errno));
		exit(1);
	}
}

Worker_Connection *shovelerWorkerConnect(int argc, char **argv, int argumentOffset, Worker_ConnectionParameters *connectionParameters)
{
	const char *launcherPrefix = "spatialos.launch:";
	if(argc - argumentOffset == 2 && g_str_has_prefix(argv[argumentOffset + 1], launcherPrefix)) {
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
		GString *pit = g_string_new("");
		const char *locatorHost = "locator.improbable.io";
		for(int i = 0; afterDeploymentNameSplit[i] != NULL; i++) {
			const char *tokenPrefix = "token=";
			const char *pitPrefix = "pit=";
			const char *environmentPrefix = "environment=";

			if(g_str_has_prefix(afterDeploymentNameSplit[i], tokenPrefix)) {
				g_string_append(authToken, afterDeploymentNameSplit[i] + strlen(tokenPrefix));
			} else if(g_str_has_prefix(afterDeploymentNameSplit[i], pitPrefix)) {
				g_string_append(pit, afterDeploymentNameSplit[i] + strlen(pitPrefix));
			} else if (g_str_has_prefix(afterDeploymentNameSplit[i], environmentPrefix)) {
				if (strcmp(afterDeploymentNameSplit[i] + strlen(environmentPrefix), "staging") == 0) {
					locatorHost = "locator-staging.improbable.io";
				} else if (strcmp(afterDeploymentNameSplit[i] + strlen(environmentPrefix), "testing") == 0) {
					locatorHost = "locator-testing.improbable.io";
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

		shovelerLogInfo("Connecting to cloud deployment...\n\tLocator host: %s\n\tProject name: %s\n\tDeployment name: %s\n\tAuth token: %s\n\tPlayer identity token: %s", locatorHost, projectName, deploymentName, authToken->str, pit->str);

		Worker_LocatorParameters locatorParameters;
		memset(&locatorParameters, 0, sizeof(Worker_LocatorParameters));
		locatorParameters.player_identity.login_token = authToken->str;
		locatorParameters.player_identity.player_identity_token = pit->str;

		Worker_Locator *locator = Worker_Locator_Create(locatorHost, /* port */ 0, &locatorParameters);

		connectionParameters->network.use_external_ip = true;
		Worker_ConnectionFuture *connectionFuture = Worker_Locator_ConnectAsync(locator, connectionParameters);
		Worker_Connection *connection = Worker_ConnectionFuture_Get(connectionFuture, /* timeoutMillis */ NULL);

		Worker_ConnectionFuture_Destroy(connectionFuture);
		Worker_Locator_Destroy(locator);
		g_string_free(authToken, true);
		g_string_free(pit, true);

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

		shovelerLogInfo("Generating authorization token.");
		char token[MAXIMUM_AUTHORIZATION_TOKEN_SIZE+1];
		generateAuthorizationToken(token, argv[0], workerId, connectionParameters->worker_type, "ShovelerTest");
		shovelerLogInfo("Authorization token(%d): %s<<", strlen(token), token);
		connectionParameters->worker_auth_token = token;

		shovelerLogInfo("Connecting to local deployment...\n\tWorker ID: %s\n\tAddress: %s:%d", workerId, hostname, port);

		Worker_ConnectionFuture *connectionFuture = Worker_ConnectDirectAsync(hostname, port, connectionParameters);
		Worker_Connection *connection = Worker_ConnectionFuture_Get(connectionFuture, /* timeoutMillis */ NULL);

		Worker_ConnectionFuture_Destroy(connectionFuture);
		g_string_free(randomWorkerId, true);

		return connection;
	}
}
