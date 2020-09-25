#ifndef SHOVELER_WORKER_COMMON_CONFIGURATION_H
#define SHOVELER_WORKER_COMMON_CONFIGURATION_H

#include <improbable/c_worker.h>
#include <shoveler/types.h>

typedef enum {
	SHOVELER_WORKER_GAME_TYPE_LIGHTS,
	SHOVELER_WORKER_GAME_TYPE_TILES,
} ShovelerWorkerGameType;

void shovelerWorkerConfigurationParseFloatFlag(Worker_Connection *connection, const char *flagName, float *outputValue);
void shovelerWorkerConfigurationParseVector3Flag(Worker_Connection *connection, const char *flagName, ShovelerVector3 *outputValue);
void shovelerWorkerConfigurationParseBoolFlag(Worker_Connection *connection, const char *flagName, bool *outputValue);
void shovelerWorkerConfigurationParseCoordinateMappingFlag(Worker_Connection *connection, const char *flagName, ShovelerCoordinateMapping *outputValue);
void shovelerWorkerConfigurationParseGameTypeFlag(Worker_Connection *connection, const char *flagName, ShovelerWorkerGameType *outputValue);

#endif
