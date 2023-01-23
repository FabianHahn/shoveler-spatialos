#ifndef SHOVELER_WORKER_COMMON_AUTHORIZATION_TOKEN_H
#define SHOVELER_WORKER_COMMON_AUTHORIZATION_TOKEN_H

#define MAXIMUM_AUTHORIZATION_TOKEN_SIZE 1024

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#define EXECUTABLE_EXTENSION ".exe"
#else
#define PATH_SEPARATOR '/'
#define EXECUTABLE_EXTENSION ""
#endif

// This is the name of the target runtime, requires the runtime to be launched
// as --runtime_id "ShovelerTest"
#define TARGET_RUNTIME_ID "ShovelerTest"

void generateAuthorizationToken(char* outputToken, const char* programPath,
	const char* workerId, const char* workerType);

#endif
