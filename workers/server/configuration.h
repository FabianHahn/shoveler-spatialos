#ifndef SHOVELER_SERVER_CONFIGURATION_H
#define SHOVELER_SERVER_CONFIGURATION_H

#include <stdbool.h> // bool

#include <improbable/c_worker.h>
#include <shoveler/configuration.h>

typedef struct {
	ShovelerWorkerGameType gameType;
} ShovelerServerConfiguration;

bool shovelerServerGetWorkerConfiguration(Worker_Connection *connection, ShovelerServerConfiguration *outputServerConfiguration);

#endif
