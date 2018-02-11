#include <assert.h> // assert
#include <stdlib.h> // malloc free

#include <shoveler/light/point.h>
#include <shoveler/light.h>
#include <shoveler/log.h>
#include <shoveler/scene.h>
#include <shoveler/spatialos/worker/view/base/position.h>
#include <shoveler/spatialos/worker/view/base/view.h>

#include "shoveler/spatialos/worker/view/visual/light.h"
#include "shoveler/spatialos/worker/view/visual/scene.h"

typedef struct {
	ShovelerLight *light;
	ShovelerSpatialosWorkerViewComponentCallback *positionCallback;
} LightComponentData;

static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *lightComponentDataPointer);
static void freeComponent(ShovelerSpatialosWorkerViewComponent *component);

bool shovelerSpatialosWorkerViewAddEntityLight(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewLightConfiguration lightConfiguration)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to add light to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewLightComponentName);
	if(component != NULL) {
		shovelerLogWarning("Trying to add light to entity %lld which already has a light, ignoring.", entityId);
		return false;
	}

	ShovelerLight *light;
	switch(lightConfiguration.type) {
		case SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_SPOT:
			shovelerLogWarning("Trying to create light with unsupported spot type, ignoring.");
			return false;
			break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_POINT:
			light = shovelerLightPointCreate((ShovelerVector3) {0.0f, 0.0f, 0.0f}, lightConfiguration.width, lightConfiguration.height, lightConfiguration.samples, lightConfiguration.ambientFactor, lightConfiguration.exponentialFactor, lightConfiguration.color);
			break;
		default:
			shovelerLogWarning("Trying to create light with unknown light type %d, ignoring.", lightConfiguration.type);
			return false;
	}

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneAddLight(scene, light);

	LightComponentData *lightComponentData = malloc(sizeof(LightComponentData));
	lightComponentData->light = light;
	lightComponentData->positionCallback = shovelerSpatialosWorkerViewEntityAddCallback(entity, shovelerSpatialosWorkerViewPositionComponentName, &positionCallback, lightComponentData);

	if (!shovelerSpatialosWorkerViewEntityAddComponent(entity, shovelerSpatialosWorkerViewLightComponentName, lightComponentData, &freeComponent)) {
		freeComponent(component);
		return false;
	}
	return true;
}

bool shovelerSpatialosWorkerViewRemoveEntityLight(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to remove light from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewLightComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to remove light from entity %lld which does not have a light, ignoring.", entityId);
		return false;
	}
	LightComponentData *lightComponentData = component->data;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneRemoveLight(scene, lightComponentData->light);

	return shovelerSpatialosWorkerViewEntityRemoveComponent(entity, shovelerSpatialosWorkerViewLightComponentName);
}

static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *lightComponentDataPointer)
{
	ShovelerSpatialosWorkerViewPosition *position = positionComponent->data;
	LightComponentData *lightComponentData = lightComponentDataPointer;
	shovelerLightUpdatePosition(lightComponentData->light, (ShovelerVector3){position->x, position->y, position->z});
}

static void freeComponent(ShovelerSpatialosWorkerViewComponent *component)
{
	assert(shovelerSpatialosWorkerViewHasScene(component->entity->view));

	LightComponentData *lightComponentData = component->data;

	ShovelerLight *light = lightComponentData->light;

	if(light != NULL) {
		ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(component->entity->view);
		shovelerSceneRemoveLight(scene, light);
	}

	shovelerLightFree(light);

	shovelerSpatialosWorkerViewEntityRemoveCallback(component->entity, shovelerSpatialosWorkerViewPositionComponentName, lightComponentData->positionCallback);

	free(lightComponentData);
}
