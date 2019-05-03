#ifndef SHOVELER_CLIENT_CONFIGURATION_H
#define SHOVELER_CLIENT_CONFIGURATION_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/game.h>
#include <shoveler/types.h>
}

struct ClientConfiguration {
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
};

ClientConfiguration getClientConfiguration(worker::Connection& connection);

#endif
