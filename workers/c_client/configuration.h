#ifndef SHOVELER_CLIENT_CONFIGURATION_H
#define SHOVELER_CLIENT_CONFIGURATION_H

#include <stdbool.h> // bool

#include <improbable/c_worker.h>
#include <shoveler/game.h>

typedef enum {
	SHOVELER_CLIENT_GAME_TYPE_LIGHTS,
	SHOVELER_CLIENT_GAME_TYPE_TILES,
} ShovelerClientGameType;

typedef struct {
	ShovelerGameControllerSettings controllerSettings;
	bool controllerLockMoveX;
	bool controllerLockMoveY;
	bool controllerLockMoveZ;
	bool controllerLockTiltX;
	bool controllerLockTiltY;
	ShovelerCoordinateMapping positionMappingX;
	ShovelerCoordinateMapping positionMappingY;
	ShovelerCoordinateMapping positionMappingZ;
	bool hidePlayerClientEntityModel;
	ShovelerClientGameType gameType;
} ShovelerClientConfiguration;

bool shovelerClientGetWorkerConfiguration(Worker_Connection *connection, ShovelerClientConfiguration *outputClientConfiguration);

#endif
