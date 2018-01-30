#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_BASE_VIEW_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_BASE_VIEW_H

#include <stdbool.h> // bool

#include <glib.h>

typedef struct {
	/** map from entity id (long long int) to entities (ShovelerSpatialosWorkerViewEntity *) */
	GHashTable *entities;
} ShovelerSpatialosWorkerView;

typedef struct {
	ShovelerSpatialosWorkerView *view;
	long long int entityId;
	/** map from string component name to (ShovelerSpatialosWorkerViewComponent *) */
	GHashTable *components;
} ShovelerSpatialosWorkerViewEntity;

struct ShovelerSpatialosWorkerViewComponentStruct; // forward declaration

typedef void (ShovelerSpatialosWorkerViewComponentFreeFunction)(struct ShovelerSpatialosWorkerViewComponentStruct *);

typedef struct ShovelerSpatialosWorkerViewComponentStruct {
	void *data;
	ShovelerSpatialosWorkerViewComponentFreeFunction *free;
} ShovelerSpatialosWorkerViewComponent;

ShovelerSpatialosWorkerView *shovelerSpatialosWorkerViewCreate();
bool shovelerSpatialosWorkerViewAddEntity(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewRemoveEntity(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewEntityAddComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, ShovelerSpatialosWorkerViewComponent *component);
bool shovelerSpatialosWorkerViewEntityRemoveComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName);
void shovelerSpatialosWorkerViewFree(ShovelerSpatialosWorkerView *view);

static inline ShovelerSpatialosWorkerViewEntity *shovelerSpatialosWorkerViewGetEntity(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	return (ShovelerSpatialosWorkerViewEntity *) g_hash_table_lookup(view->entities, &entityId);
}

static inline ShovelerSpatialosWorkerViewComponent *shovelerSpatialosWorkerViewEntityGetComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *component)
{
	return (ShovelerSpatialosWorkerViewComponent *) g_hash_table_lookup(entity->components, component);
}

#endif
