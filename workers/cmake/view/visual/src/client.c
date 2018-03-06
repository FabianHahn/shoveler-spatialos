#include <assert.h> // assert
#include <stdlib.h> // malloc free

#include <shoveler/spatialos/worker/view/base/position.h>
#include <shoveler/spatialos/worker/view/base/view.h>
#include <shoveler/spatialos/log.h>
#include <shoveler/controller.h>
#include <shoveler/types.h>
#include <shoveler/model.h>

#include "shoveler/spatialos/worker/view/visual/client.h"
#include "shoveler/spatialos/worker/view/visual/controller.h"
#include "shoveler/spatialos/worker/view/visual/model.h"

typedef struct {
	ShovelerSpatialosWorkerViewEntity *entity;
	double x;
	double y;
	double z;
	ShovelerSpatialosWorkerViewComponentCallback *positionCallback;
	ShovelerSpatialosWorkerViewComponentCallback *modelCallback;
	ShovelerControllerMoveCallback *moveCallback;
} ClientComponentData;

static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *clientComponentDataPointer);
static void modelCallback(ShovelerSpatialosWorkerViewComponent *modelComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *unused);
static void moveCallback(ShovelerController *controller, ShovelerVector3 position, void *clientComponentDataPointer);
static void freeComponent(ShovelerSpatialosWorkerViewComponent *component);

bool shovelerSpatialosWorkerViewAddEntityClient(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	assert(shovelerSpatialosWorkerViewHasController(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to add client to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewClientComponentName);
	if(component != NULL) {
		shovelerSpatialosLogWarning("Trying to add client to entity %lld which already has a client, ignoring.", entityId);
		return false;
	}

	ClientComponentData *clientComponentData = malloc(sizeof(ClientComponentData));
	clientComponentData->entity = entity;
	clientComponentData->x = 0;
	clientComponentData->y = 0;
	clientComponentData->z = 0;
	clientComponentData->positionCallback = NULL;
	clientComponentData->moveCallback = NULL;

	if (!shovelerSpatialosWorkerViewEntityAddComponent(entity, shovelerSpatialosWorkerViewClientComponentName, clientComponentData, &freeComponent)) {
		freeComponent(component);
		return false;
	}
	return true;
}

bool shovelerSpatialosWorkerViewDelegateClient(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to delegate client on existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewClientComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to delegate client on entity %lld which does not have a client, ignoring.", entityId);
		return false;
	}
	ClientComponentData *clientComponentData = component->data;

	clientComponentData->x = 0;
	clientComponentData->y = 0;
	clientComponentData->z = 0;

	clientComponentData->positionCallback = shovelerSpatialosWorkerViewEntityAddCallback(entity, shovelerSpatialosWorkerViewPositionComponentName, positionCallback, clientComponentData);
	clientComponentData->modelCallback = shovelerSpatialosWorkerViewEntityAddCallback(entity, shovelerSpatialosWorkerViewModelComponentName, modelCallback, NULL);

	ShovelerController *controller = shovelerSpatialosWorkerViewGetController(view);
	clientComponentData->moveCallback = shovelerControllerAddMoveCallback(controller, moveCallback, clientComponentData);

	return shovelerSpatialosWorkerViewDelegateComponent(entity, shovelerSpatialosWorkerViewClientComponentName);
}

bool shovelerSpatialosWorkerViewUndelegateClient(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to undelegate client on existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewClientComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to undelegate client on entity %lld which does not have a client, ignoring.", entityId);
		return false;
	}
	ClientComponentData *clientComponentData = component->data;

	shovelerSpatialosWorkerViewEntityRemoveCallback(component->entity, shovelerSpatialosWorkerViewPositionComponentName, clientComponentData->positionCallback);
	clientComponentData->positionCallback = NULL;
	shovelerSpatialosWorkerViewEntityRemoveCallback(component->entity, shovelerSpatialosWorkerViewModelComponentName, clientComponentData->modelCallback);
	clientComponentData->modelCallback = NULL;

	ShovelerController *controller = shovelerSpatialosWorkerViewGetController(component->entity->view);
	shovelerControllerRemoveMoveCallback(controller, clientComponentData->moveCallback);
	clientComponentData->moveCallback = NULL;

	return shovelerSpatialosWorkerViewUndelegateComponent(entity, shovelerSpatialosWorkerViewClientComponentName);
}

bool shovelerSpatialosWorkerViewRemoveEntityClient(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerSpatialosLogWarning("Trying to remove client from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewClientComponentName);
	if(component == NULL) {
		shovelerSpatialosLogWarning("Trying to remove client from entity %lld which does not have a client, ignoring.", entityId);
		return false;
	}
	ClientComponentData *clientComponentData = component->data;

	return shovelerSpatialosWorkerViewEntityRemoveComponent(entity, shovelerSpatialosWorkerViewClientComponentName);
}

static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *clientComponentDataPointer)
{
	ShovelerSpatialosWorkerViewPosition *position = positionComponent->data;
	ClientComponentData *clientComponentData = clientComponentDataPointer;

	if(callbackType == VIEW_COMPONENT_CALLBACK_ADD) {
		shovelerSpatialosLogInfo("Resetting client component tracked position to (%.2f, %.2f, %.2f).", position->x, position->y, position->z);
		clientComponentData->x = position->x;
		clientComponentData->y = position->y;
		clientComponentData->z = position->z;
	}
}

static void modelCallback(ShovelerSpatialosWorkerViewComponent *modelComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *unused)
{
	ShovelerSpatialosWorkerViewModel *model = modelComponent->data;

	if(callbackType == VIEW_COMPONENT_CALLBACK_ADD) {
		shovelerSpatialosLogInfo("Hiding model for entity with authoritative client component.");
		model->model->visible = false;
	} else if(callbackType == VIEW_COMPONENT_CALLBACK_REMOVE) {
		shovelerSpatialosLogInfo("Reenabling model visibility for entity with unauthoritative client component.");
		model->model->visible = false;
	}
}

static void moveCallback(ShovelerController *controller, ShovelerVector3 position, void *clientComponentDataPointer)
{
	ClientComponentData *clientComponentData = clientComponentDataPointer;

	clientComponentData->x = position.values[0];
	clientComponentData->y = position.values[1];
	clientComponentData->z = position.values[2];

	shovelerSpatialosWorkerViewRequestPositionUpdate(clientComponentData->entity->view, clientComponentData->entity->entityId, clientComponentData->x, clientComponentData->y, clientComponentData->z);
}

static void freeComponent(ShovelerSpatialosWorkerViewComponent *component)
{
	assert(shovelerSpatialosWorkerViewHasController(component->entity->view));

	ClientComponentData *clientComponentData = component->data;

	if(clientComponentData->positionCallback != NULL) {
		shovelerSpatialosWorkerViewEntityRemoveCallback(component->entity, shovelerSpatialosWorkerViewPositionComponentName, clientComponentData->positionCallback);
	}

	if(clientComponentData->modelCallback != NULL) {
		shovelerSpatialosWorkerViewEntityRemoveCallback(component->entity, shovelerSpatialosWorkerViewModelComponentName, clientComponentData->modelCallback);
	}

	if(clientComponentData->moveCallback != NULL) {
		ShovelerController *controller = shovelerSpatialosWorkerViewGetController(component->entity->view);
		shovelerControllerRemoveMoveCallback(controller, clientComponentData->moveCallback);
	}

	free(clientComponentData);
}
