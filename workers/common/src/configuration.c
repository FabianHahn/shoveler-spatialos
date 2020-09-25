#include "shoveler/configuration.h"

#include <stdio.h> // sscanf
#include <stdlib.h> // atof
#include <string.h> // strdup

#include <shoveler/log.h>

static void workerFlagCallback(void *targetPointer, const char *value);

void shovelerWorkerConfigurationParseFloatFlag(Worker_Connection *connection, const char *flagName, float *outputValue)
{
	char *stringValue;
	Worker_Connection_GetWorkerFlag(connection, flagName, &stringValue, workerFlagCallback);
	if(stringValue == NULL) {
		return;
	}

	*outputValue = atof(stringValue);

	shovelerLogInfo("Parsed configuration flag '%s' with value %f.", flagName, *outputValue);
	free(stringValue);
}

void shovelerWorkerConfigurationParseVector3Flag(Worker_Connection *connection, const char *flagName, ShovelerVector3 *outputValue)
{
	char *stringValue;
	Worker_Connection_GetWorkerFlag(connection, flagName, &stringValue, workerFlagCallback);
	if(stringValue == NULL) {
		return;
	}

	sscanf(stringValue, "%f %f %f", &outputValue->values[0], &outputValue->values[1], &outputValue->values[2]);

	shovelerLogInfo("Parsed client configuration flag '%s' with value (%f, %f, %f).", flagName, outputValue->values[0], outputValue->values[1], outputValue->values[2]);
	free(stringValue);
}

void shovelerWorkerConfigurationParseBoolFlag(Worker_Connection *connection, const char *flagName, bool *outputValue)
{
	char *stringValue;
	Worker_Connection_GetWorkerFlag(connection, flagName, &stringValue, workerFlagCallback);
	if(stringValue == NULL) {
		return;
	}

	if(strcmp(stringValue, "true") == 0) {
		*outputValue = true;
	} else if(strcmp(stringValue, "false") == 0) {
		*outputValue = false;
	} else {
		free(stringValue);
		return;
	}

	shovelerLogInfo("Parsed client configuration flag '%s' with value %s.", flagName, stringValue);
	free(stringValue);
}

void shovelerWorkerConfigurationParseCoordinateMappingFlag(Worker_Connection *connection, const char *flagName, ShovelerCoordinateMapping *outputValue)
{
	char *stringValue;
	Worker_Connection_GetWorkerFlag(connection, flagName, &stringValue, workerFlagCallback);
	if(stringValue == NULL) {
		return;
	}

	if(strcmp(stringValue, "+x") == 0) {
		*outputValue = SHOVELER_COORDINATE_MAPPING_POSITIVE_X;
	} else if(strcmp(stringValue, "+y") == 0) {
		*outputValue = SHOVELER_COORDINATE_MAPPING_POSITIVE_Y;
	} else if(strcmp(stringValue, "+z") == 0) {
		*outputValue = SHOVELER_COORDINATE_MAPPING_POSITIVE_Z;
	} else if(strcmp(stringValue, "-x") == 0) {
		*outputValue = SHOVELER_COORDINATE_MAPPING_NEGATIVE_X;
	} else if(strcmp(stringValue, "-y") == 0) {
		*outputValue = SHOVELER_COORDINATE_MAPPING_NEGATIVE_Y;
	} else if(strcmp(stringValue, "-z") == 0) {
		*outputValue = SHOVELER_COORDINATE_MAPPING_NEGATIVE_Z;
	} else {
		free(stringValue);
		return;
	}

	shovelerLogInfo("Parsed client configuration flag '%s' with value %s.", flagName, stringValue);
	free(stringValue);
}

void shovelerWorkerConfigurationParseGameTypeFlag(Worker_Connection *connection, const char *flagName, ShovelerWorkerGameType *outputValue)
{
	char *stringValue;
	Worker_Connection_GetWorkerFlag(connection, flagName, &stringValue, workerFlagCallback);
	if(stringValue == NULL) {
		return;
	}

	if(strcmp(stringValue, "lights") == 0) {
		*outputValue = SHOVELER_WORKER_GAME_TYPE_LIGHTS;
	} else if (strcmp(stringValue, "tiles") == 0) {
		*outputValue = SHOVELER_WORKER_GAME_TYPE_TILES;
	} else {
		free(stringValue);
		return;
	}

	shovelerLogInfo("Parsed client configuration flag '%s' with value '%s'.", flagName, stringValue);
	free(stringValue);
}

static void workerFlagCallback(void *targetPointer, const char *value)
{
	char **target = (char **) targetPointer;

	if (value != NULL) {
		*target = strdup(value);
	} else {
		*target = NULL;
	}
}
