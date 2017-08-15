#ifndef SHOVELER_SPATIAL_OS_WORKER_VIEW_H
#define SHOVELER_SPATIAL_OS_WORKER_VIEW_H

#include <stdbool.h> // bool

#include <glib.h>

typedef struct {
	/** map from entity id (long long int) to entities (ShovelerSpatialOsWorkerViewEntity *) */
	GHashTable *entities;
} ShovelerSpatialOsWorkerView;

typedef struct {
	ShovelerSpatialOsWorkerView *view;
	long long int entityId;
	/** map from component id (int) to components (ShovelerSpatialOsWorkerViewComponent *) */
	GHashTable *components;
} ShovelerSpatialOsWorkerViewEntity;

typedef struct {
	ShovelerSpatialOsWorkerViewEntity *entity;
	int componentId;
	bool authoritative;
	void *data;
} ShovelerSpatialOsWorkerViewEntityComponent;

ShovelerSpatialOsWorkerView *shovelerSpatialOsWorkerViewCreate();
bool shovelerSpatialOsWorkerViewAddEntity(ShovelerSpatialOsWorkerView *view, long long int entityId);
bool shovelerSpatialOsWorkerViewRemoveEntity(ShovelerSpatialOsWorkerView *view, long long int entityId);
bool shovelerSpatialOsWorkerViewAddEntityComponent(ShovelerSpatialOsWorkerView *view, long long int entityId, int componentId);
bool shovelerSpatialOsWorkerViewRemoveEntityComponent(ShovelerSpatialOsWorkerView *view, long long int entityId, int componentId);
void shovelerSpatialOsWorkerViewFree(ShovelerSpatialOsWorkerView *view);

static inline ShovelerSpatialOsWorkerViewEntity *shovelerSpatialOsWorkerViewGetEntity(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	return (ShovelerSpatialOsWorkerViewEntity *) g_hash_table_lookup(view->entities, &entityId);
}

static inline ShovelerSpatialOsWorkerViewEntityComponent *shovelerSpatialOsWorkerViewEntityGetComponent(ShovelerSpatialOsWorkerViewEntity *entity, int componentId)
{
	return (ShovelerSpatialOsWorkerViewEntityComponent *) g_hash_table_lookup(entity->components, &componentId);
}

static inline ShovelerSpatialOsWorkerViewEntityComponent *shovelerSpatialOsWorkerViewGetEntityComponent(ShovelerSpatialOsWorkerView *view, long long int entityId, int componentId)
{
	ShovelerSpatialOsWorkerViewEntity *entity = shovelerSpatialOsWorkerViewGetEntity(view, entityId);
	if(entity == NULL) {
		return NULL;
	}

	return shovelerSpatialOsWorkerViewEntityGetComponent(entity, componentId);
}

#endif
