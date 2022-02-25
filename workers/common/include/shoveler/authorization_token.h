#ifndef SHOVELER_WORKER_COMMON_AUTHORIZATION_TOKEN_H
#define SHOVELER_WORKER_COMMON_AUTHORIZATION_TOKEN_H

#define MAXIMUM_AUTHORIZATION_TOKEN_SIZE 1024

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

// This is the name of the target runtime, requires the runtime to be launched
// as --runtime_id "ShovelerTest"
#define TARGET_RUNTIME_ID "ShovelerTest"

#endif
