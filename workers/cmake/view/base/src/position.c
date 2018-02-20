#include <stdlib.h> // malloc free

#include <shoveler/spatialos/log.h>

#include "shoveler/spatialos/worker/view/base/position.h"
#include "shoveler/spatialos/worker/view/base/view.h"

static void freeComponent(ShovelerSpatialosWorkerViewComponent *component);

bool shovelerSpatialosWorkerViewAddEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to add position to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
	if(component != NULL) {
		shovelerSpatialosLogWarning("Trying to add position to entity %lld which already has a position, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewPosition *position = malloc(sizeof(ShovelerSpatialosWorkerViewPosition));
	position->x = x;
	position->y = y;
	position->z = z;
	position->requestUpdate = NULL;
	position->requestUpdateUserData = NULL;

	if (!shovelerSpatialosWorkerViewEntityAddComponent(entity, shovelerSpatialosWorkerViewPositionComponentName, position, &freeComponent)) {
		freeComponent(component);
		return false;
	}
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to update position for non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to update position for entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewPosition *position = component->data;

	position->x = x;
	position->y = y;
	position->z = z;

	return shovelerSpatialosWorkerViewEntityUpdateComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
}

bool shovelerSpatialosWorkerViewDelegatePosition(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewPositionRequestUpdateFunction *requestUpdateFunction, void *userData)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to delegate position for non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to delegate position for entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewPosition *position = component->data;

	position->requestUpdate = requestUpdateFunction;
	position->requestUpdateUserData = userData;

	return shovelerSpatialosWorkerViewDelegateComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
}

bool shovelerSpatialosWorkerViewUndelegatePosition(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to undelegate position for non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to undelegate position for entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewPosition *position = component->data;

	position->requestUpdate = NULL;
	position->requestUpdateUserData = NULL;

	return shovelerSpatialosWorkerViewUndelegateComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
}

bool shovelerSpatialosWorkerViewRequestPositionUpdate(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to request position update for non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to request position update for entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewPosition *position = component->data;

	if (!component->authoritative) {
		shovelerSpatialosLogWarning("Trying to request position update for entity %lld for which this worker is not authoritative, ignoring.", entityId);
		return false;
	}

	position->requestUpdate(component, x, y, z, position->requestUpdateUserData);
	return true;
}

bool shovelerSpatialosWorkerViewRemoveEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to remove position from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to remove position from entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}

	return shovelerSpatialosWorkerViewEntityRemoveComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
}


static void freeComponent(ShovelerSpatialosWorkerViewComponent *component)
{
	ShovelerSpatialosWorkerViewPosition *position = component->data;
	free(position);
}
