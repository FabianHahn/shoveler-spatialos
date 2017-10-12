#include <stdlib.h> // malloc free

#include <shoveler/drawable/cube.h>
#include <shoveler/drawable/point.h>
#include <shoveler/drawable/quad.h>
#include <shoveler/light/point.h>
#include <shoveler/material/color.h>
#include <shoveler/material/particle.h>
#include <shoveler/material/texture.h>
#include <shoveler/log.h>

#include "worker_view.h"

static ShovelerDrawable *createDrawable(ShovelerSpatialOsWorkerView *view, ShovelerSpatialOsWorkerViewDrawableConfiguration configuration);
static ShovelerMaterial *createMaterial(ShovelerSpatialOsWorkerView *view, ShovelerSpatialOsWorkerViewMaterialConfiguration configuration);
static void updateEntityPosition(ShovelerSpatialOsWorkerViewEntity *entity);
static void freeEntity(void *entityPointer);

ShovelerSpatialOsWorkerView *shovelerSpatialOsWorkerViewCreate(ShovelerScene *scene)
{
	ShovelerSpatialOsWorkerView *view = malloc(sizeof(ShovelerSpatialOsWorkerView));
	view->scene = scene;
	view->entities = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, freeEntity);
	view->cube = shovelerDrawableCubeCreate();
	view->quad = shovelerDrawableQuadCreate();
	view->point = shovelerDrawablePointCreate();

	return view;
}

bool shovelerSpatialOsWorkerViewAddEntity(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = malloc(sizeof(ShovelerSpatialOsWorkerViewEntity));
	entity->view = view;
	entity->entityId = entityId;
	entity->position = NULL;
	entity->model = NULL;
	entity->light = NULL;

	if(!g_hash_table_insert(view->entities, &entity->entityId, entity)) {
		shovelerLogWarning("Trying to add already existing entity %lld to world view, ignoring.", entityId);
		freeEntity(entity);
		return false;
	} else {
		return true;
	}
}

bool shovelerSpatialOsWorkerViewRemoveEntity(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	return g_hash_table_remove(view->entities, &entityId);
}

bool shovelerSpatialOsWorkerViewAddEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerVector3 position)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to add position to non existing entity %lld, ignoring.", entityId);
		return false;
	}
	
	if(entity->position != NULL) {
		shovelerLogWarning("Trying to add position to entity %lld which already has a position, ignoring.", entityId);
		return false;
	}

	entity->position = malloc(sizeof(ShovelerVector3));
	*entity->position = position;
	updateEntityPosition(entity);
	return true;
}

bool shovelerSpatialOsWorkerViewUpdateEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerVector3 position)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update position for non existing entity %lld, ignoring.", entityId);
		return false;
	}

	if(entity->position == NULL) {
		shovelerLogWarning("Trying to update position for entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}

	*entity->position = position;
	updateEntityPosition(entity);
	return true;
}

bool shovelerSpatialOsWorkerViewRemoveEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to remove position from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	if(entity->position == NULL) {
		shovelerLogWarning("Trying to remove position from entity %lld which does not have a position, ignoring.", entityId);
		return false;
	}

	free(entity->position);
	entity->position = NULL;
	return true;
}

bool shovelerSpatialOsWorkerViewAddEntityModel(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerSpatialOsWorkerViewDrawableConfiguration drawableConfiguration, ShovelerSpatialOsWorkerViewMaterialConfiguration materialConfiguration)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to add model to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	if(entity->model != NULL) {
		shovelerLogWarning("Trying to add model to entity %lld which already has a model, ignoring.", entityId);
		return false;
	}

	ShovelerDrawable *drawable = createDrawable(view, drawableConfiguration);
	if(drawable == NULL) {
		shovelerLogWarning("Trying to add model to entity %lld but failed to create drawable, ignoring.", entityId);
		return false;
	}

	ShovelerMaterial *material = createMaterial(view, materialConfiguration);
	if(material == NULL) {
		shovelerLogWarning("Trying to add model to entity %lld but failed to create material, ignoring.", entityId);
		return false;
	}

	entity->model = shovelerModelCreate(drawable, material);
	shovelerSceneAddModel(view->scene, entity->model);
	updateEntityPosition(entity);
	return true;
}

bool shovelerSpatialOsWorkerViewUpdateEntityModel(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerSpatialOsWorkerViewDrawableConfiguration *optionalDrawableConfiguration, ShovelerSpatialOsWorkerViewMaterialConfiguration *optionalMaterialConfiguration)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update model for non existing entity %lld, ignoring.", entityId);
		return false;
	}

	if(entity->model == NULL) {
		shovelerLogWarning("Trying to update model to entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}

	ShovelerDrawable *drawable = entity->model->drawable;
	ShovelerMaterial *material = entity->model->material;

	shovelerSceneRemoveModel(view->scene, entity->model);
	entity->model = NULL;

	if (optionalDrawableConfiguration != NULL) {
		drawable = createDrawable(view, *optionalDrawableConfiguration);
	}

	if (optionalMaterialConfiguration != NULL) {
		shovelerMaterialFree(material);
		material = createMaterial(view, *optionalMaterialConfiguration);
	}

	entity->model = shovelerModelCreate(drawable, material);
	shovelerSceneAddModel(view->scene, entity->model);
	updateEntityPosition(entity);
	return true;
}

bool shovelerSpatialOsWorkerViewRemoveEntityModel(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to remove model from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	if(entity->model == NULL) {
		shovelerLogWarning("Trying to remove model from entity %lld which does not have a model, ignoring.", entityId);
		return false;
	}

	shovelerSceneRemoveModel(view->scene, entity->model);
	entity->model = NULL;
	return true;
}

bool shovelerSpatialOsWorkerViewAddEntityLight(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerSpatialOsWorkerViewLightConfiguration lightConfiguration)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to add light to non existing entity %lld, ignoring.", entityId);
		return false;
	}

	if(entity->light != NULL) {
		shovelerLogWarning("Trying to add light to entity %lld which already has a light, ignoring.", entityId);
		return false;
	}

	switch(lightConfiguration.type) {
		case SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_SPOT:
			shovelerLogWarning("Trying to create light with unsupported spot type, ignoring.");
			return false;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_POINT: {
			entity->light = shovelerLightPointCreate((ShovelerVector3) {0.0f, 0.0f, 0.0f}, lightConfiguration.width, lightConfiguration.height, lightConfiguration.samples, lightConfiguration.ambientFactor, lightConfiguration.exponentialFactor, lightConfiguration.color);
		}
		break;
		default:
			shovelerLogWarning("Trying to create light with unknown light type %d, ignoring.", lightConfiguration.type);
			return false;
	}

	shovelerSceneAddLight(view->scene, entity->light);
	updateEntityPosition(entity);
	return true;
}

bool shovelerSpatialOsWorkerViewRemoveEntityLight(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to remove light from non existing entity %lld, ignoring.", entityId);
		return false;
	}

	if(entity->light == NULL) {
		shovelerLogWarning("Trying to remove light from entity %lld which does not have a light, ignoring.", entityId);
		return false;
	}

	shovelerSceneRemoveLight(view->scene, entity->light);
	entity->light = NULL;
	return true;
}

void shovelerSpatialOsWorkerViewFree(ShovelerSpatialOsWorkerView *view)
{
	g_hash_table_destroy(view->entities);
	free(view);
}

static ShovelerDrawable *createDrawable(ShovelerSpatialOsWorkerView *view, ShovelerSpatialOsWorkerViewDrawableConfiguration configuration)
{
	switch (configuration.type) {
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_CUBE:
			return view->cube;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_QUAD:
			return view->quad;
		break;
		case SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_POINT:
			return view->point;
		break;
		default:
			shovelerLogWarning("Trying to create drawable with unknown type %d, ignoring.", configuration.type);
			return NULL;
	}
}

static ShovelerMaterial *createMaterial(ShovelerSpatialOsWorkerView *view, ShovelerSpatialOsWorkerViewMaterialConfiguration configuration)
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

static void updateEntityPosition(ShovelerSpatialOsWorkerViewEntity *entity)
{
	if(entity->position == NULL) {
		return;
	}

	if(entity->model != NULL) {
		entity->model->translation = *entity->position;
		shovelerModelUpdateTransformation(entity->model);
	}

	if(entity->light != NULL) {
		shovelerLightUpdatePosition(entity->light, *entity->position);
	}
}

static void freeEntity(void *entityPointer)
{
	ShovelerSpatialOsWorkerViewEntity *entity = entityPointer;
	free(entity->position);

	if(entity->model != NULL) {
		shovelerSceneRemoveModel(entity->view->scene, entity->model);
	}

	if(entity->light != NULL) {
		shovelerSceneRemoveLight(entity->view->scene, entity->light);
	}

	free(entity);
}
