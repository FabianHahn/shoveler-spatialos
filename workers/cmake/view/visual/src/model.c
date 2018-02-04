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

#include "shoveler/spatialos/worker/view/visual/model.h"
#include "shoveler/spatialos/worker/view/visual/scene.h"

static ShovelerDrawable *getDrawable(ShovelerSpatialosWorkerView *view, ShovelerSpatialosWorkerViewDrawableConfiguration configuration);
static ShovelerMaterial *createMaterial(ShovelerSpatialosWorkerView *view, ShovelerSpatialosWorkerViewMaterialConfiguration configuration);
static void updatePositionIfAvailable(ShovelerSpatialosWorkerViewEntity *entity, ShovelerModel *model);
static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *modelPointer);
static void freeComponent(ShovelerSpatialosWorkerViewComponent *component);

bool shovelerSpatialosWorkerViewAddEntityModel(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewModelConfiguration modelConfiguration)
{
	assert(shovelerSpatialosWorkerViewHasScene(view));

	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to add model to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
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

	if(!shovelerSpatialosWorkerViewEntityAddCallback(entity, "position", &positionCallback, model)) {
		freeComponent(component);
		return false;
	}

	if (!shovelerSpatialosWorkerViewEntityAddComponent(entity, "model", model, &freeComponent)) {
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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model drawable of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}

	ShovelerModel *oldModel = component->data;
	ShovelerDrawable *drawable = getDrawable(view, drawableConfiguration);
	ShovelerModel *model = shovelerModelCreate(drawable, oldModel->material);
	model->rotation = oldModel->rotation;
	model->scale = oldModel->scale;
	model->visible = oldModel->visible;
	model->emitter = oldModel->emitter;
	model->screenspace = oldModel->screenspace;
	model->castsShadow = oldModel->castsShadow;
	model->polygonMode = oldModel->polygonMode;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneRemoveModel(scene, oldModel);
	shovelerSceneAddModel(scene, model);

	updatePositionIfAvailable(entity, model);
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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model material of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}

	ShovelerModel *oldModel = component->data;
	ShovelerMaterial *oldMaterial = oldModel->material;
	ShovelerMaterial *material = createMaterial(view, materialConfiguration);
	ShovelerModel *model = shovelerModelCreate(oldModel->drawable, material);
	model->rotation = oldModel->rotation;
	model->scale = oldModel->scale;
	model->visible = oldModel->visible;
	model->emitter = oldModel->emitter;
	model->screenspace = oldModel->screenspace;
	model->castsShadow = oldModel->castsShadow;
	model->polygonMode = oldModel->polygonMode;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneRemoveModel(scene, oldModel);
	shovelerMaterialFree(oldMaterial);
	shovelerSceneAddModel(scene, model);

	updatePositionIfAvailable(entity, model);
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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model rotation of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerModel *model = component->data;

	model->rotation = rotation;
	shovelerModelUpdateTransformation(model);
	return shovelerSpatialosWorkerViewEntityUpdateComponent(entity, "model");
}

bool shovelerSpatialosWorkerViewUpdateEntityModelScale(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerVector3 scale)
{
	ShovelerSpatialosWorkerViewEntity *entity = shovelerSpatialosWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model scale of non existing entity %lld, ignoring.", entityId);
		return false;
	}

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model scale of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerModel *model = component->data;

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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model visibility of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerModel *model = component->data;

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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model emitter of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerModel *model = component->data;

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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model screenspace of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerModel *model = component->data;

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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to update model polygon mode of entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerModel *model = component->data;

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

	ShovelerSpatialosWorkerViewComponent *component = shovelerSpatialosWorkerViewEntityGetComponent(entity, "model");
	if(component == NULL) {
		shovelerLogWarning("Trying to remove model from entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}
	ShovelerModel *model = component->data;

	ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(view);
	shovelerSceneRemoveModel(scene, model);

	return shovelerSpatialosWorkerViewEntityRemoveComponent(entity, "model");
}

void shovelerSpatialosWorkerViewFree(ShovelerSpatialosWorkerView *view)
{
	g_hash_table_destroy(view->entities);
	free(view);
}

static ShovelerDrawable *getDrawable(ShovelerSpatialosWorkerView *view, ShovelerSpatialosWorkerViewDrawableConfiguration configuration)
{
	switch (configuration.type) {
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_CUBE:
			// return view->cube;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_QUAD:
			// return view->quad;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_POINT:
			// return view->point;
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

static void updatePositionIfAvailable(ShovelerSpatialosWorkerViewEntity *entity, ShovelerModel *model)
{
	ShovelerSpatialosWorkerViewComponent *positionComponent = shovelerSpatialosWorkerViewEntityGetComponent(entity, "position");
	if(positionComponent != NULL) {
		positionCallback(positionComponent, VIEW_COMPONENT_CALLBACK_USER, model);
	}
}

static void positionCallback(ShovelerSpatialosWorkerViewComponent *positionComponent, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *modelPointer)
{
	ShovelerSpatialosWorkerViewPosition *position = positionComponent->data;
	ShovelerModel *model = modelPointer;

	model->translation.values[0] = position->x;
	model->translation.values[1] = position->y;
	model->translation.values[2] = position->z;
	shovelerModelUpdateTransformation(model);
}

static void freeComponent(ShovelerSpatialosWorkerViewComponent *component)
{
	assert(shovelerSpatialosWorkerViewHasScene(component->entity->view));

	ShovelerModel *model = component->data;

	if(model != NULL) {
		ShovelerScene *scene = shovelerSpatialosWorkerViewGetScene(component->entity->view);
		shovelerSceneRemoveModel(scene, model);
	}

	shovelerModelFree(model);
}
