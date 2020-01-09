#include <cstring> // memcpy strdup

#include <improbable/standard_library.h>

#include "position.h"

extern "C" {
#include <shoveler/view/position.h>
#include <shoveler/component.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
}

using improbable::Coordinates;
using improbable::Position;

static ShovelerVector3 mapPositionCoordinates(const Coordinates& coordinates, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
static ShovelerVector3 mapPositionCoordinatesVector(ShovelerVector3 coordinatesVector, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);

void registerPositionCallbacks(worker::Connection& connection, worker::Dispatcher& dispatcher, ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	dispatcher.OnAddComponent<Position>([&, view, mappingX, mappingY, mappingZ](const worker::AddComponentOp<Position>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerVector3 coordinates = mapPositionCoordinates(op.Data.coords(), mappingX, mappingY, mappingZ);
		shovelerViewEntityAddPosition(entity, coordinates);
	});

	dispatcher.OnComponentUpdate<Position>([&, view, mappingX, mappingY, mappingZ](const worker::ComponentUpdateOp<Position>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerComponent *component = shovelerViewEntityGetPositionComponent(entity);

		if(op.Update.coords()) {
			ShovelerVector3 coordinates = mapPositionCoordinates(*op.Update.coords(), mappingX, mappingY, mappingZ);
			shovelerComponentUpdateCanonicalConfigurationOptionVector3(component, SHOVELER_COMPONENT_POSITION_OPTION_ID_COORDINATES, coordinates);
		}
	});

	dispatcher.OnAuthorityChange<Position>([&, view](const worker::AuthorityChangeOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		if (op.Authority == worker::Authority::kAuthoritative) {
			shovelerViewEntityDelegate(entity, shovelerComponentTypeIdPosition);
		} else if (op.Authority == worker::Authority::kNotAuthoritative) {
            shovelerViewEntityUndelegate(entity, shovelerComponentTypeIdPosition);
		}
	});

	dispatcher.OnRemoveComponent<Position>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemovePosition(entity);
	});
}

void requestPositionUpdate(worker::Connection& connection, ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ, ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value)
{
    assert(component->type->id == shovelerComponentTypeIdPosition);
    assert(configurationOption->type == SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR3);

    // TODO: inverse mapping here?
    ShovelerVector3 coordinates = mapPositionCoordinatesVector(value->vector3Value, mappingX, mappingY, mappingZ);

    Position::Update positionUpdate;
    positionUpdate.set_coords({coordinates.values[0], coordinates.values[1], coordinates.values[2]});
    connection.SendComponentUpdate<Position>(component->entityId, positionUpdate);
}

ShovelerVector3 getEntitySpatialOsPosition(ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ, worker::EntityId entityId)
{
	ShovelerVector3 spatialOsPosition = shovelerVector3(0.0f, 0.0f, 0.0f);

	ShovelerViewEntity *entity = shovelerViewGetEntity(view, entityId);
	if(entity != NULL) {
		ShovelerComponent *position = shovelerViewEntityGetPositionComponent(entity);
		if(position != NULL) {
		    const ShovelerVector3 *coordinates = shovelerComponentGetPositionCoordinates(position);
			spatialOsPosition = mapPositionCoordinatesVector(*coordinates, mappingX, mappingY, mappingZ);
		}
	}

	return spatialOsPosition;
}

static ShovelerVector3 mapPositionCoordinates(const Coordinates& coordinates, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	ShovelerVector3 coordinatesVector = shovelerVector3((float) coordinates.x(), (float) coordinates.y(), (float) coordinates.z());
	return mapPositionCoordinatesVector(coordinatesVector, mappingX, mappingY, mappingZ);
}

static ShovelerVector3 mapPositionCoordinatesVector(ShovelerVector3 coordinatesVector, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
    float mappedX = shovelerCoordinateMap(coordinatesVector, mappingX);
    float mappedY = shovelerCoordinateMap(coordinatesVector, mappingY);
    float mappedZ = shovelerCoordinateMap(coordinatesVector, mappingZ);

    return shovelerVector3(mappedX, mappedY, mappedZ);
}
