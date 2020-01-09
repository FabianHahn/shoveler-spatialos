#ifndef SHOVELER_CLIENT_POSITION_H
#define SHOVELER_CLIENT_POSITION_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
#include <shoveler/types.h>
}

void registerPositionCallbacks(worker::Connection& connection, worker::Dispatcher& dispatcher, ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
void requestPositionUpdate(worker::Connection& connection, ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value);
ShovelerVector3 getEntitySpatialOsPosition(ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ, worker::EntityId entityId);

#endif
