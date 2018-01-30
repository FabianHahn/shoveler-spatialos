#include <stdlib.h> // malloc free
#include <string.h> // strdup

#include "shoveler/spatialos/worker/view/base/view.h"

static void freeEntity(void *entityPointer);
static void freeComponent(void *componentPointer);

ShovelerSpatialosWorkerView *shovelerSpatialosWorkerViewCreate()
{
	ShovelerSpatialosWorkerView *view = malloc(sizeof(ShovelerSpatialosWorkerView));
	view->entities = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, freeEntity);

	return view;
}

bool shovelerSpatialosWorkerViewAddEntity(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = malloc(sizeof(ShovelerSpatialosWorkerViewEntity));
	entity->view = view;
	entity->entityId = entityId;
	entity->components = g_hash_table_new_full(&g_str_hash, &g_str_equal, &free, &freeComponent);

	if(!g_hash_table_insert(view->entities, &entity->entityId, entity)) {
		freeEntity(entity);
		return false;
	} else {
		return true;
	}
}

bool shovelerSpatialosWorkerViewRemoveEntity(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	return g_hash_table_remove(view->entities, &entityId);
}

bool shovelerSpatialosWorkerViewEntityAddComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, ShovelerSpatialosWorkerViewComponent *component)
{
	char *key = strdup(componentName);
	if(!g_hash_table_insert(entity->components, key, component)) {
		free(key);
		return false;
	} else {
		return true;
	}
}

bool shovelerSpatialosWorkerViewEntityRemoveComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName)
{
	return g_hash_table_remove(entity->components, componentName);
}

static void freeEntity(void *entityPointer)
{
	ShovelerSpatialosWorkerViewEntity *entity = entityPointer;
	g_hash_table_destroy(entity->components);
	free(entity);
}

static void freeComponent(void *componentPointer)
{
	ShovelerSpatialosWorkerViewComponent *component = componentPointer;
	component->free(component);
	free(component);
}