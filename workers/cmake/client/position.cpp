#include <cstring> // memcpy strdup

#include <improbable/standard_library.h>

#include "position.h"

extern "C" {
#include <shoveler/view/position.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
}

using improbable::Coordinates;
using improbable::Position;

static struct PositionUpdateRequestContext {
	worker::Connection *connection;
	ShovelerCoordinateMapping mappingX;
	ShovelerCoordinateMapping mappingY;
	ShovelerCoordinateMapping mappingZ;
} updateContext;

static ShovelerVector3 mapPositionCoordinates(const Coordinates& coordinates, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
static void requestPositionUpdate(ShovelerViewComponent *component, double x, double y, double z, void *positionUpdateRequestContextPointer);

void registerPositionCallbacks(worker::Connection& connection, worker::Dispatcher& dispatcher, ShovelerView *view, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	dispatcher.OnAddComponent<Position>([&, view, mappingX, mappingY, mappingZ](const worker::AddComponentOp<Position>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerVector3 coordinates = mapPositionCoordinates(op.Data.coords(), mappingX, mappingY, mappingZ);
		shovelerViewEntityAddPosition(entity, coordinates.values[0], coordinates.values[1], coordinates.values[2]);
	});

	dispatcher.OnComponentUpdate<Position>([&, view, mappingX, mappingY, mappingZ](const worker::ComponentUpdateOp<Position>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		if(op.Update.coords()) {
			ShovelerVector3 coordinates = mapPositionCoordinates(*op.Update.coords(), mappingX, mappingY, mappingZ);
			shovelerViewEntityUpdatePosition(entity, coordinates.values[0], coordinates.values[1], coordinates.values[2]);
		}
	});

	updateContext.connection = &connection;
	updateContext.mappingX = mappingX;
	updateContext.mappingY = mappingY;
	updateContext.mappingZ = mappingZ;
	dispatcher.OnAuthorityChange<Position>([&, view](const worker::AuthorityChangeOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		if (op.Authority == worker::Authority::kAuthoritative) {
			shovelerViewEntityDelegatePosition(entity, requestPositionUpdate, &updateContext);
		} else if (op.Authority == worker::Authority::kNotAuthoritative) {
			shovelerViewEntityUndelegatePosition(entity);
		}
	});


	dispatcher.OnRemoveComponent<Position>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemovePosition(entity);
	});
}

static ShovelerVector3 mapPositionCoordinates(const Coordinates& coordinates, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	ShovelerVector3 coordinatesVector = shovelerVector3((float) coordinates.x(), (float) coordinates.y(), (float) coordinates.z());
	float mappedX = shovelerCoordinateMap(coordinatesVector, mappingX);
	float mappedY = shovelerCoordinateMap(coordinatesVector, mappingY);
	float mappedZ = shovelerCoordinateMap(coordinatesVector, mappingZ);

	return shovelerVector3(mappedX, mappedY, mappedZ);
}

static void requestPositionUpdate(ShovelerViewComponent *component, double x, double y, double z, void *positionUpdateRequestContextPointer)
{
	PositionUpdateRequestContext *context = (PositionUpdateRequestContext *) positionUpdateRequestContextPointer;

	ShovelerVector3 coordinates = mapPositionCoordinates({x, y, z}, context->mappingX, context->mappingY, context->mappingZ);

	Position::Update positionUpdate;
	positionUpdate.set_coords({coordinates.values[0], coordinates.values[1], coordinates.values[2]});
	context->connection->SendComponentUpdate<Position>(component->entity->entityId, positionUpdate);
}
