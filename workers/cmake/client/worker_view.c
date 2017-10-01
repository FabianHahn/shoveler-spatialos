#include <stdlib.h> // malloc free

#include <log.h>

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
	entity->position = NULL;

	if(!g_hash_table_insert(view->entities, &entity->entityId, entity)) {
		shovelerLogWarning("Trying to add already existing entity %lld to world view, ignoring...", entityId);
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
		shovelerLogWarning("Trying to add position to non existing entity %lld, ignoring...", entityId);
		return false;
	}
	
	if(entity->position != NULL) {
		shovelerLogWarning("Trying to add position to entity %lld which already has a position, ignoring...", entityId);
		return false;
	}

	entity->position = malloc(sizeof(ShovelerVector3));
	*entity->position = position;
	return true;
}

bool shovelerSpatialOsWorkerViewUpdateEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerVector3 position)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to update position for non existing entity %lld, ignoring...", entityId);
		return false;
	}

	if(entity->position == NULL) {
		shovelerLogWarning("Trying to update position for entity %lld which does not have a position, ignoring...", entityId);
		return false;
	}

	*entity->position = position;
	return true;
}

bool shovelerSpatialOsWorkerViewRemoveEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		shovelerLogWarning("Trying to remove position from non existing entity %lld, ignoring...", entityId);
		return false;
	}

	if(entity->position == NULL) {
		shovelerLogWarning("Trying to remove position from entity %lld which does not have a position, ignoring...", entityId);
		return false;
	}

	free(entity->position);
	entity->position = NULL;
	return true;
}

void shovelerSpatialOsWorkerViewFree(ShovelerSpatialOsWorkerView *view)
{
	g_hash_table_destroy(view->entities);
	free(view);
}

static void freeEntity(void *entityPointer)
{
	ShovelerSpatialOsWorkerViewEntity *entity = entityPointer;
	free(entity->position);
	free(entity);
}
