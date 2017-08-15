#include <stdlib.h> // malloc free

#include "worker_view.h"

static void freeEntity(void *entityPointer);
static void freeComponent(void *componentPointer);

ShovelerSpatialOsWorkerView *shovelerSpatialOsWorkerViewCreate()
{
	ShovelerSpatialOsWorkerView *view = malloc(sizeof(ShovelerSpatialOsWorkerView));
	view->entities = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, freeEntity);

	return view;
}

bool shovelerSpatialOsWorkerViewAddEntity(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = malloc(sizeof(ShovelerSpatialOsWorkerViewEntity));
	entity->view = view;
	entity->entityId = entityId;
	entity->components = g_hash_table_new_full(g_int_hash, g_int_equal, NULL, freeComponent);

	if(!g_hash_table_insert(view->entities, &entityId, entity)) {
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

bool shovelerSpatialOsWorkerViewAddEntityComponent(ShovelerSpatialOsWorkerView *view, long long int entityId, int componentId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		return NULL;
	}

	ShovelerSpatialOsWorkerViewEntityComponent *component = malloc(sizeof(ShovelerSpatialOsWorkerViewEntityComponent));
	component->entity = entity;
	component->componentId = componentId;
	component->authoritative = false;
	component->data = NULL;

	if(!g_hash_table_insert(entity->components, &componentId, component)) {
		freeComponent(component);
		return false;
	} else {
		return true;
	}
}

bool shovelerSpatialOsWorkerViewRemoveComponent(ShovelerSpatialOsWorkerView *view, long long int entityId, int componentId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		return NULL;
	}

	return g_hash_table_remove(entity->components, &componentId);
}

void shovelerSpatialOsWorkerViewFree(ShovelerSpatialOsWorkerView *view)
{
	g_hash_table_destroy(view->entities);
	free(view);
}

static void freeEntity(void *entityPointer)
{
	ShovelerSpatialOsWorkerViewEntity *entity = entityPointer;
	g_hash_table_destroy(entity->components);
	free(entity);
}

static void freeComponent(void *componentPointer)
{
	ShovelerSpatialOsWorkerViewEntityComponent *component = componentPointer;
	free(component);
}