#include "shoveler/authorization_token.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <shoveler/log.h>

void generateAuthorizationToken(char* outputToken, const char* programPath,
	const char* workerId, const char* workerType) {
	char programParentPath[4096];
	char jwtmakerPath[4500];
	char privateKeyPath[4500];
	
	strcpy(programParentPath, programPath);
	
	char* programPath_end = strrchr(programPath, PATH_SEPARATOR);
	if(programPath_end == NULL) {
		strcpy(programParentPath, ".");
	} else {
		programParentPath[programPath_end - programPath] = '\0';
	}
	snprintf(jwtmakerPath, sizeof(jwtmakerPath), "%s%cjwtmaker", 
		programParentPath, PATH_SEPARATOR);
	snprintf(privateKeyPath, sizeof(jwtmakerPath), "%s%cprivate_key.pem", 
		programParentPath, PATH_SEPARATOR);

	char jwtmakerCommand[10000];
	snprintf(jwtmakerCommand, sizeof(jwtmakerCommand), 
		"%s -worker_id %s -worker_type %s -runtime_id %s -pem_private_key_file %s",
		jwtmakerPath,
		workerId, workerType, TARGET_RUNTIME_ID,
		privateKeyPath);
	
	FILE* pipe = popen(jwtmakerCommand, "r");
	if(pipe != NULL) {
		if(fgets(outputToken, MAXIMUM_AUTHORIZATION_TOKEN_SIZE, pipe) == NULL) {
			shovelerLogError("Failed to read authorization token: %s\n", strerror(errno));
			exit(1);
		}
		// Remove the extra newline
		int tokenSize = strlen(outputToken);
		if(tokenSize > 0 && outputToken[tokenSize-1] == '\n') {
			outputToken[tokenSize-1] = '\0';
		}
	}
	else {
		shovelerLogError("Failed to open authorization token helper %s: %s\n", jwtmakerPath, strerror(errno));
		exit(1);
	}
}
