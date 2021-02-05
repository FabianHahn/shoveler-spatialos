#ifndef SHOVELER_WORKER_COMMON_CONFIGURATION_H
#define SHOVELER_WORKER_COMMON_CONFIGURATION_H

#include <improbable/c_worker.h>
#include <shoveler/types.h>

typedef enum {
	SHOVELER_WORKER_GAME_TYPE_LIGHTS,
	SHOVELER_WORKER_GAME_TYPE_TILES,
} ShovelerWorkerGameType;

bool shovelerWorkerConfigurationParseFloatFlag(Worker_Connection *connection, const char *flagName, float *outputValue);
bool shovelerWorkerConfigurationParseIntFlag(Worker_Connection *connection, const char *flagName, int *outputValue);
bool shovelerWorkerConfigurationParseVector3Flag(Worker_Connection *connection, const char *flagName, ShovelerVector3 *outputValue);
bool shovelerWorkerConfigurationParseBoolFlag(Worker_Connection *connection, const char *flagName, bool *outputValue);
bool shovelerWorkerConfigurationParseCoordinateMappingFlag(Worker_Connection *connection, const char *flagName, ShovelerCoordinateMapping *outputValue);
bool shovelerWorkerConfigurationParseGameTypeFlag(Worker_Connection *connection, const char *flagName, ShovelerWorkerGameType *outputValue);

#endif
