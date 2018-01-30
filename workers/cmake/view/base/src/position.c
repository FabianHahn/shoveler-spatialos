#include <stdlib.h> // malloc free

#include "shoveler/spatialos/worker/view/base/position.h"
#include "shoveler/spatialos/worker/view/base/view.h"

static void freeComponent(ShovelerSpatialosWorkerViewComponent *component);

bool shovelerSpatialosWorkerViewAddEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		// shovelerLogWarning("Trying to add position to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "position");
	if(component != NULL) {
		// shovelerLogWarning("Trying to add position to entity %lld which already has a position, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewPosition *position = malloc(sizeof(ShovelerSpatialosWorkerViewPosition));
	position->x = x;
	position->y = y;
	position->z = z;

	component = malloc(sizeof(ShovelerSpatialosWorkerViewComponent));
	component->data = position;
	component->free = &freeComponent;
	if (!shovelerSpatialosWorkerViewEntityAddComponent(entity, "model", component)) {
		freeComponent(component);
		return false;
	}
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		// shovelerLogWarning("Trying to update position for non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "position");
	if(component == NULL) {
		// shovelerLogWarning("Trying to update position for entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewPosition *position = component->data;

	position->x = x;
	position->y = y;
	position->z = z;

	// updateEntityPosition(entity);
	return true;
}

bool shovelerSpatialosWorkerViewRemoveEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		// shovelerLogWarning("Trying to remove position from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "position");
	if(component == NULL) {
		// shovelerLogWarning("Trying to remove position from entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}

	return shovelerSpatialosWorkerViewEntityRemoveComponent(entity, "model");
}


static void freeComponent(ShovelerSpatialosWorkerViewComponent *component)
{
	ShovelerSpatialosWorkerViewPosition *position = component->data;
	free(position);
}