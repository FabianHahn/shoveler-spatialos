#ifndef SHOVELER_SPATIAL_OS_WORKER_VIEW_H
#define SHOVELER_SPATIAL_OS_WORKER_VIEW_H

#include <stdbool.h> // bool

#include <glib.h>

#include <types.h>

typedef struct {
	/** map from entity id (long long int) to entities (ShovelerSpatialOsWorkerViewEntity *) */
	GHashTable *entities;
} ShovelerSpatialOsWorkerView;

typedef struct {
	ShovelerSpatialOsWorkerView *view;
	long long int entityId;
	ShovelerVector3 *position;
} ShovelerSpatialOsWorkerViewEntity;

ShovelerSpatialOsWorkerView *shovelerSpatialOsWorkerViewCreate();
bool shovelerSpatialOsWorkerViewAddEntity(ShovelerSpatialOsWorkerView *view, long long int entityId);
bool shovelerSpatialOsWorkerViewRemoveEntity(ShovelerSpatialOsWorkerView *view, long long int entityId);
bool shovelerSpatialOsWorkerViewAddEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerVector3 position);
bool shovelerSpatialOsWorkerViewUpdateEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerVector3 position);
bool shovelerSpatialOsWorkerViewRemoveEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId);
void shovelerSpatialOsWorkerViewFree(ShovelerSpatialOsWorkerView *view);

static inline ShovelerSpatialOsWorkerViewEntity *shovelerSpatialOsWorkerViewGetEntity(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	return (ShovelerSpatialOsWorkerViewEntity *) g_hash_table_lookup(view->entities, &entityId);
}

#endif
