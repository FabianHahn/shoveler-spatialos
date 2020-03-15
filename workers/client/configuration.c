#include <stdio.h> // sscanf
#include <stdlib.h> // atof
#include <string.h> // strdup

#include <shoveler/log.h>

#include "configuration.h"

static void parseFloatFlag(Worker_Connection *connection, const char *flagName, float *outputValue);
static void parseVector3Flag(Worker_Connection *connection, const char *flagName, ShovelerVector3 *outputValue);
static void parseBoolFlag(Worker_Connection *connection, const char *flagName, bool *outputValue);
static void parseCoordinateMappingFlag(Worker_Connection *connection, const char *flagName, ShovelerCoordinateMapping *outputValue);
static void parseGameTypeFlag(Worker_Connection *connection, const char *flagName, ShovelerClientGameType *outputValue);
static void workerFlagCallback(void *targetPointer, const char *value);

bool shovelerClientGetWorkerConfiguration(Worker_Connection *connection, ShovelerClientConfiguration *outputClientConfiguration)
{
	outputClientConfiguration->controllerSettings.frame.position = shovelerVector3(0, 0, 0);
	outputClientConfiguration->controllerSettings.frame.direction = shovelerVector3(0, 0, 1);
	outputClientConfiguration->controllerSettings.frame.up = shovelerVector3(0, 1, 0);
	outputClientConfiguration->controllerLockMoveX = false;
	outputClientConfiguration->controllerLockMoveY = false;
	outputClientConfiguration->controllerLockMoveZ = false;
	outputClientConfiguration->controllerLockTiltX = false;
	outputClientConfiguration->controllerLockTiltY = false;
	outputClientConfiguration->controllerSettings.moveFactor = 2.0f;
	outputClientConfiguration->controllerSettings.tiltFactor = 0.0005f;
	outputClientConfiguration->controllerSettings.boundingBoxSize2 = 0.0f;
	outputClientConfiguration->controllerSettings.boundingBoxSize3 = 0.0f;
	outputClientConfiguration->positionMappingX = SHOVELER_COORDINATE_MAPPING_POSITIVE_X;
	outputClientConfiguration->positionMappingY = SHOVELER_COORDINATE_MAPPING_POSITIVE_Y;
	outputClientConfiguration->positionMappingZ = SHOVELER_COORDINATE_MAPPING_POSITIVE_Z;
	outputClientConfiguration->hidePlayerClientEntityModel = true;
	outputClientConfiguration->gameType = SHOVELER_CLIENT_GAME_TYPE_LIGHTS;

	parseVector3Flag(connection, "controller_frame_position", &outputClientConfiguration->controllerSettings.frame.position);
	parseVector3Flag(connection, "controller_frame_direction", &outputClientConfiguration->controllerSettings.frame.direction);
	parseVector3Flag(connection, "controller_frame_up", &outputClientConfiguration->controllerSettings.frame.up);
	parseBoolFlag(connection, "controller_lock_move_x", &outputClientConfiguration->controllerLockMoveX);
	parseBoolFlag(connection, "controller_lock_move_y", &outputClientConfiguration->controllerLockMoveY);
	parseBoolFlag(connection, "controller_lock_move_z", &outputClientConfiguration->controllerLockMoveZ);
	parseBoolFlag(connection, "controller_lock_tilt_x", &outputClientConfiguration->controllerLockTiltX);
	parseBoolFlag(connection, "controller_lock_tilt_y", &outputClientConfiguration->controllerLockTiltY);
	parseFloatFlag(connection, "controller_move_factor", &outputClientConfiguration->controllerSettings.moveFactor);
	parseFloatFlag(connection, "controller_tilt_factor", &outputClientConfiguration->controllerSettings.tiltFactor);
	parseFloatFlag(connection, "controller_bounding_box_2d_size", &outputClientConfiguration->controllerSettings.boundingBoxSize2);
	parseFloatFlag(connection, "controller_bounding_box_3d_size", &outputClientConfiguration->controllerSettings.boundingBoxSize3);
	parseCoordinateMappingFlag(connection, "position_mapping_x", &outputClientConfiguration->positionMappingX);
	parseCoordinateMappingFlag(connection, "position_mapping_y", &outputClientConfiguration->positionMappingY);
	parseCoordinateMappingFlag(connection, "position_mapping_z", &outputClientConfiguration->positionMappingZ);
	parseBoolFlag(connection, "hide_player_client_entity_model", &outputClientConfiguration->hidePlayerClientEntityModel);
	parseGameTypeFlag(connection, "game_type", &outputClientConfiguration->gameType);

	return true;
}

static void parseFloatFlag(Worker_Connection *connection, const char *flagName, float *outputValue)
{
	char *stringValue;
	Worker_Connection_GetWorkerFlag(connection, flagName, &stringValue, workerFlagCallback);
	if(stringValue == NULL) {
		return;
	}

	*outputValue = atof(stringValue);

	shovelerLogInfo("Parsed client configuration flag '%s' with value %f.", flagName, *outputValue);
	free(stringValue);
}

static void parseVector3Flag(Worker_Connection *connection, const char *flagName, ShovelerVector3 *outputValue)
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

static void parseBoolFlag(Worker_Connection *connection, const char *flagName, bool *outputValue)
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

static void parseCoordinateMappingFlag(Worker_Connection *connection, const char *flagName, ShovelerCoordinateMapping *outputValue)
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

static void parseGameTypeFlag(Worker_Connection *connection, const char *flagName, ShovelerClientGameType *outputValue)
{
	char *stringValue;
	Worker_Connection_GetWorkerFlag(connection, flagName, &stringValue, workerFlagCallback);
	if(stringValue == NULL) {
		return;
	}

	if(strcmp(stringValue, "lights") == 0) {
		*outputValue = SHOVELER_CLIENT_GAME_TYPE_LIGHTS;
	} else if (strcmp(stringValue, "tiles") == 0) {
		*outputValue = SHOVELER_CLIENT_GAME_TYPE_TILES;
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
