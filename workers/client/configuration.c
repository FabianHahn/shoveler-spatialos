#include <stdio.h> // sscanf
#include <stdlib.h> // atof
#include <string.h> // strdup

#include <shoveler/log.h>
#include <shoveler/configuration.h>

#include "configuration.h"

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
	outputClientConfiguration->gameType = SHOVELER_WORKER_GAME_TYPE_LIGHTS;

	shovelerWorkerConfigurationParseVector3Flag(connection, "controller_frame_position", &outputClientConfiguration->controllerSettings.frame.position);
	shovelerWorkerConfigurationParseVector3Flag(connection, "controller_frame_direction", &outputClientConfiguration->controllerSettings.frame.direction);
	shovelerWorkerConfigurationParseVector3Flag(connection, "controller_frame_up", &outputClientConfiguration->controllerSettings.frame.up);
	shovelerWorkerConfigurationParseBoolFlag(connection, "controller_lock_move_x", &outputClientConfiguration->controllerLockMoveX);
	shovelerWorkerConfigurationParseBoolFlag(connection, "controller_lock_move_y", &outputClientConfiguration->controllerLockMoveY);
	shovelerWorkerConfigurationParseBoolFlag(connection, "controller_lock_move_z", &outputClientConfiguration->controllerLockMoveZ);
	shovelerWorkerConfigurationParseBoolFlag(connection, "controller_lock_tilt_x", &outputClientConfiguration->controllerLockTiltX);
	shovelerWorkerConfigurationParseBoolFlag(connection, "controller_lock_tilt_y", &outputClientConfiguration->controllerLockTiltY);
	shovelerWorkerConfigurationParseFloatFlag(connection, "controller_move_factor", &outputClientConfiguration->controllerSettings.moveFactor);
	shovelerWorkerConfigurationParseFloatFlag(connection, "controller_tilt_factor", &outputClientConfiguration->controllerSettings.tiltFactor);
	shovelerWorkerConfigurationParseFloatFlag(connection, "controller_bounding_box_2d_size", &outputClientConfiguration->controllerSettings.boundingBoxSize2);
	shovelerWorkerConfigurationParseFloatFlag(connection, "controller_bounding_box_3d_size", &outputClientConfiguration->controllerSettings.boundingBoxSize3);
	shovelerWorkerConfigurationParseCoordinateMappingFlag(connection, "position_mapping_x", &outputClientConfiguration->positionMappingX);
	shovelerWorkerConfigurationParseCoordinateMappingFlag(connection, "position_mapping_y", &outputClientConfiguration->positionMappingY);
	shovelerWorkerConfigurationParseCoordinateMappingFlag(connection, "position_mapping_z", &outputClientConfiguration->positionMappingZ);
	shovelerWorkerConfigurationParseBoolFlag(connection, "hide_player_client_entity_model", &outputClientConfiguration->hidePlayerClientEntityModel);
	shovelerWorkerConfigurationParseGameTypeFlag(connection, "game_type", &outputClientConfiguration->gameType);

	return true;
}
