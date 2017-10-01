#ifndef SHOVELER_SPATIAL_OS_WORKER_VIEW_H
#define SHOVELER_SPATIAL_OS_WORKER_VIEW_H

#include <stdbool.h> // bool

#include <glib.h>

#include <drawable.h>
#include <material.h>
#include <model.h>
#include <types.h>

typedef enum {
	SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_CUBE,
	SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_QUAD,
} ShovelerSpatialOsWorkerViewDrawableType;

typedef struct {
	ShovelerSpatialOsWorkerViewDrawableType type;
} ShovelerSpatialOsWorkerViewDrawableConfiguration;

typedef enum {
	SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_COLOR,
	SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_TEXTURE
} ShovelerSpatialOsWorkerViewMaterialType;

typedef struct {
	ShovelerSpatialOsWorkerViewMaterialType type;
	ShovelerVector3 color;
	const char *texture;
} ShovelerSpatialOsWorkerViewMaterialConfiguration;

typedef struct {
	/** map from entity id (long long int) to entities (ShovelerSpatialOsWorkerViewEntity *) */
	GHashTable *entities;
	ShovelerDrawable *cube;
	ShovelerDrawable *quad;
} ShovelerSpatialOsWorkerView;

typedef struct {
	ShovelerSpatialOsWorkerView *view;
	long long int entityId;
	ShovelerVector3 *position;
	ShovelerModel *model;
} ShovelerSpatialOsWorkerViewEntity;

ShovelerSpatialOsWorkerView *shovelerSpatialOsWorkerViewCreate();
bool shovelerSpatialOsWorkerViewAddEntity(ShovelerSpatialOsWorkerView *view, long long int entityId);
bool shovelerSpatialOsWorkerViewRemoveEntity(ShovelerSpatialOsWorkerView *view, long long int entityId);
bool shovelerSpatialOsWorkerViewAddEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerVector3 position);
bool shovelerSpatialOsWorkerViewUpdateEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerVector3 position);
bool shovelerSpatialOsWorkerViewRemoveEntityPosition(ShovelerSpatialOsWorkerView *view, long long int entityId);
bool shovelerSpatialOsWorkerViewAddEntityModel(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerSpatialOsWorkerViewDrawableConfiguration drawableConfiguration, ShovelerSpatialOsWorkerViewMaterialConfiguration materialConfiguration);
bool shovelerSpatialOsWorkerViewUpdateEntityModel(ShovelerSpatialOsWorkerView *view, long long int entityId, ShovelerSpatialOsWorkerViewDrawableConfiguration *optionalDrawableConfiguration, ShovelerSpatialOsWorkerViewMaterialConfiguration *optionalMaterialConfiguration);
bool shovelerSpatialOsWorkerViewRemoveEntityModel(ShovelerSpatialOsWorkerView *view, long long int entityId);
void shovelerSpatialOsWorkerViewFree(ShovelerSpatialOsWorkerView *view);

static inline ShovelerSpatialOsWorkerViewEntity *shovelerSpatialOsWorkerViewGetEntity(ShovelerSpatialOsWorkerView *view, long long int entityId)
{
	return (ShovelerSpatialOsWorkerViewEntity *) g_hash_table_lookup(view->entities, &entityId);
}

#endif
