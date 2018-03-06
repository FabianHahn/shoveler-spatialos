#include <assert.h> // assert
#include <stdlib.h> // malloc free

#include <shoveler/drawable/cube.h>
#include <shoveler/drawable/point.h>
#include <shoveler/drawable/quad.h>
#include <shoveler/material/color.h>
#include <shoveler/material/particle.h>
#include <shoveler/material/texture.h>
#include <shoveler/spatialos/worker/view/base/position.h>
#include <shoveler/spatialos/worker/view/base/view.h>
#include <shoveler/log.h>
#include <shoveler/model.h>

#include "shoveler/spatialos/worker/view/visual/drawables.h"
#include "shoveler/spatialos/worker/view/visual/model.h"
#include "shoveler/spatialos/worker/view/visual/scene.h"

static ShovelerDrawable *getDrawable(ShovelerSpatialosWorkerView *view, ShovelerSpatialosWorkerViewDrawableConfiguration configuration);
static ShovelerMaterial *createMaterial(ShovelerSpatialosWorkerView *view, ShovelerSpatialosWorkerViewMaterialConfiguration configuration);
static void updatePositionIfAvailable(ShovelerSpatialosWorkerViewEntity *entity, ShovelerSpatialosWorkerViewModel *modelComponentData);
static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *modelComponentDataPointer);
static void freeComponent(ShovelerSpatialosWorkerViewComponent *component);

bool shovelerSpatialosWorkerViewAddEntityModel(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewModelConfiguration modelConfiguration)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to add model to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component != NULL) {
		shovelerLogWarning("Trying to add model to entity %lld which already has a model, ignoring.", entityId);
		return false;
	}

	ShovelerDrawable *drawable = getDrawable(view, modelConfiguration.drawable);
	if(drawable == NULL) {
		shovelerLogWarning("Trying to add model to entity %lld but failed to create drawable, ignoring.", entityId);
		return false;
	}

	ShovelerMaterial *material = createMaterial(view, modelConfiguration.material);
	if(material == NULL) {
		shovelerLogWarning("Trying to add model to entity %lld but failed to create material, ignoring.", entityId);
		return false;
	}

	ShovelerModel *model = shovelerModelCreate(drawable, material);
	model->rotation = modelConfiguration.rotation;
	model->scale = modelConfiguration.scale;
	model->visible = modelConfiguration.visible;
	model->emitter = modelConfiguration.emitter;
	model->screenspace = modelConfiguration.screenspace;
	model->castsShadow = modelConfiguration.castsShadow;
	model->polygonMode = modelConfiguration.polygonMode;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneAddModel(scene, model);

	ShovelerSpatialosWorkerViewModel *modelComponentData = malloc(sizeof(ShovelerSpatialosWorkerViewModel));
	modelComponentData->model = model;
	modelComponentData->positionCallback = shovelerSpatialosWorkerViewEntityAddCallback(entity, shovelerSpatialosWorkerViewPositionComponentName, &positionCallback, modelComponentData);

	if (!shovelerSpatialosWorkerViewEntityAddComponent(entity, shovelerSpatialosWorkerViewModelComponentName, modelComponentData, &freeComponent)) {
		freeComponent(component);
		return false;
	}
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityModelDrawable(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewDrawableConfiguration drawableConfiguration)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model drawable of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model drawable of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *oldModel = modelComponentData->model;
	ShovelerDrawable *drawable = getDrawable(view, drawableConfiguration);
	modelComponentData->model = shovelerModelCreate(drawable, oldModel->material);
	modelComponentData->model->rotation = oldModel->rotation;
	modelComponentData->model->scale = oldModel->scale;
	modelComponentData->model->visible = oldModel->visible;
	modelComponentData->model->emitter = oldModel->emitter;
	modelComponentData->model->screenspace = oldModel->screenspace;
	modelComponentData->model->castsShadow = oldModel->castsShadow;
	modelComponentData->model->polygonMode = oldModel->polygonMode;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneRemoveModel(scene, oldModel);
	shovelerSceneAddModel(scene, modelComponentData->model);

	updatePositionIfAvailable(entity, modelComponentData);
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityModelMaterial(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewMaterialConfiguration materialConfiguration)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model material of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model material of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *oldModel = modelComponentData->model;
	ShovelerMaterial *oldMaterial = oldModel->material;
	ShovelerMaterial *material = createMaterial(view, materialConfiguration);
	modelComponentData->model = shovelerModelCreate(oldModel->drawable, material);
	modelComponentData->model->rotation = oldModel->rotation;
	modelComponentData->model->scale = oldModel->scale;
	modelComponentData->model->visible = oldModel->visible;
	modelComponentData->model->emitter = oldModel->emitter;
	modelComponentData->model->screenspace = oldModel->screenspace;
	modelComponentData->model->castsShadow = oldModel->castsShadow;
	modelComponentData->model->polygonMode = oldModel->polygonMode;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneRemoveModel(scene, oldModel);
	shovelerMaterialFree(oldMaterial);
	shovelerSceneAddModel(scene, modelComponentData->model);

	updatePositionIfAvailable(entity, modelComponentData);
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityModelRotation(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerVector3 rotation)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model rotation of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model rotation of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;
	model->rotation = rotation;
	shovelerModelUpdateTransformation(model);
	return shovelerSpatialosWorkerViewEntityUpdateComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
}

bool shovelerSpatialosWorkerViewUpdateEntityModelScale(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerVector3 scale)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model scale of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model scale of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;
	model->scale = scale;
	shovelerModelUpdateTransformation(model);
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityModelVisible(ShovelerSpatialosWorkerView *view, long long int entityId, bool visible)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model visibility of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model visibility of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;
	model->visible = visible;
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityModelEmitter(ShovelerSpatialosWorkerView *view, long long int entityId, bool emitter)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model emitter of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model emitter of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;
	model->emitter = emitter;
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityModelScreenspace(ShovelerSpatialosWorkerView *view, long long int entityId, bool screenspace)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model screenspace of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model screenspace of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;
	model->screenspace = screenspace;
	return true;
}

bool shovelerSpatialosWorkerViewUpdateEntityModelPolygonMode(ShovelerSpatialosWorkerView *view, long long int entityId, GLuint polygonMode)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model polygon mode of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to update model polygon mode of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;
	model->polygonMode = polygonMode;
	return true;
}

bool shovelerSpatialosWorkerViewRemoveEntityModel(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to remove model from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
	if(component == NULL) {
		shovelerLogWarning("Trying to remove model from entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneRemoveModel(scene, model);

	return shovelerSpatialosWorkerViewEntityRemoveComponent(entity, shovelerSpatialosWorkerViewModelComponentName);
}

static ShovelerDrawable *getDrawable(ShovelerSpatialosWorkerView *view, ShovelerSpatialosWorkerViewDrawableConfiguration configuration)
{
	assert(shovelerSpatialosWorkerViewHasDrawables(view));
	ShovelerSpatialosWorkerViewDrawables *drawables = shovelerSpatialosWorkerViewGetDrawables(view);

	switch (configuration.type) {
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_CUBE:
			return drawables->cube;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_QUAD:
			return drawables->quad;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_POINT:
			return drawables->point;
		break;
		default:
			shovelerLogWarning("Trying to create drawable with unknown type %d, ignoring.", configuration.type);
			return NULL;
	}
}

static ShovelerMaterial *createMaterial(ShovelerSpatialosWorkerView *view, ShovelerSpatialosWorkerViewMaterialConfiguration configuration)
{
	switch (configuration.type) {
		case SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_COLOR:
			return shovelerMaterialColorCreate(configuration.color);
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_TEXTURE:
			shovelerLogWarning("Trying to create model with unsupported texture type, ignoring.", configuration.type);
			return NULL;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_PARTICLE:
			return shovelerMaterialParticleCreate(configuration.color);
		break;
		default:
			shovelerLogWarning("Trying to create model with unknown material type %d, ignoring.", configuration.type);
			return NULL;
	}
}

static void updatePositionIfAvailable(ShovelerSpatialosWorkerViewEntity *entity, ShovelerSpatialosWorkerViewModel *modelComponentData)
{
	ShovelerSpatialosWorkerViewComponent *positionComponent = shovelerSpatialosWorkerViewEntityGetComponent(entity, shovelerSpatialosWorkerViewPositionComponentName);
	if(positionComponent != NULL) {
		positionCallback(positionComponent, VIEW_COMPONENT_CALLBACK_USER, modelComponentData);
	}
}

static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *modelComponentDataPointer)
{
	ShovelerSpatialosWorkerViewPosition *position = positionComponent->data;
	ShovelerSpatialosWorkerViewModel *modelComponentData = modelComponentDataPointer;
	ShovelerModel *model = modelComponentData->model;

	model->translation.values[0] = position->x;
	model->translation.values[1] = position->y;
	model->translation.values[2] = position->z;
	shovelerModelUpdateTransformation(model);
}

static void freeComponent(ShovelerSpatialosWorkerViewComponent *component)
{
	assert(shovelerSpatialosWorkerViewHasScene(component->entity->view));

	ShovelerSpatialosWorkerViewModel *modelComponentData = component->data;

	ShovelerModel *model = modelComponentData->model;
	if(model != NULL) {
		ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(component->entity->view);
		shovelerSceneRemoveModel(scene, model);
	}

	shovelerSpatialosWorkerViewEntityRemoveCallback(component->entity, shovelerSpatialosWorkerViewPositionComponentName, modelComponentData->positionCallback);

	free(modelComponentData);
}
