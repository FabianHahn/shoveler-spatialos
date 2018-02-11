#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_BASE_VIEW_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_BASE_VIEW_H

#include <stdbool.h> // bool

#include <glib.h>

struct ShovelerSpatialosWorkerViewComponentStruct; // forward declaration

typedef struct {
	/** map from entity id (long long int) to entities (ShovelerSpatialosWorkerViewEntity *) */
	GHashTable *entities;
	/** map from string target name to target type */
	GHashTable *targets;
} ShovelerSpatialosWorkerView;

typedef enum {
	VIEW_COMPONENT_CALLBACK_ADD,
	VIEW_COMPONENT_CALLBACK_REMOVE,
	VIEW_COMPONENT_CALLBACK_UPDATE,
	VIEW_COMPONENT_CALLBACK_USER,
} ShovelerSpatialosWorkerViewComponentCallbackType;

typedef void (ShovelerSpatialosWorkerViewComponentCallbackFunction)(struct ShovelerSpatialosWorkerViewComponentStruct *component, ShovelerSpatialosWorkerViewComponentCallbackType callbackType, void *userData);

typedef struct {
	ShovelerSpatialosWorkerViewComponentCallbackFunction *function;
	void *userData;
} ShovelerSpatialosWorkerViewComponentCallback;

typedef struct {
	ShovelerSpatialosWorkerView *view;
	long long int entityId;
	/** map from string component name to (ShovelerSpatialosWorkerViewComponent *) */
	GHashTable *components;
	/** map from string component name to (GQueue *) of (ShovelerSpatialosWorkerViewComponentCallback *) */
	GHashTable *callbacks;
} ShovelerSpatialosWorkerViewEntity;

typedef void (ShovelerSpatialosWorkerViewComponentFreeFunction)(struct ShovelerSpatialosWorkerViewComponentStruct *);

typedef struct ShovelerSpatialosWorkerViewComponentStruct {
	ShovelerSpatialosWorkerViewEntity *entity;
	char *name;
	void *data;
	ShovelerSpatialosWorkerViewComponentFreeFunction *free;
} ShovelerSpatialosWorkerViewComponent;

ShovelerSpatialosWorkerView *shovelerSpatialosWorkerViewCreate();
bool shovelerSpatialosWorkerViewAddEntity(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewRemoveEntity(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewEntityAddComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, void *data, ShovelerSpatialosWorkerViewComponentFreeFunction *freeFunction);
bool shovelerSpatialosWorkerViewEntityUpdateComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName);
bool shovelerSpatialosWorkerViewEntityRemoveComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName);
ShovelerSpatialosWorkerViewComponentCallback *shovelerSpatialosWorkerViewEntityAddCallback(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, ShovelerSpatialosWorkerViewComponentCallbackFunction *function, void *userData);
bool shovelerSpatialosWorkerViewEntityRemoveCallback(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, ShovelerSpatialosWorkerViewComponentCallback *callback);
/** Sets a target that is expected to be freed by the caller. */
bool shovelerSpatialosWorkerViewSetTarget(ShovelerSpatialosWorkerView *view, const char *targetName, void *target);
void shovelerSpatialosWorkerViewFree(ShovelerSpatialosWorkerView *view);

static inline ShovelerSpatialosWorkerViewEntity *shovelerSpatialosWorkerViewGetEntity(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	return (ShovelerSpatialosWorkerViewEntity *) g_hash_table_lookup(view->entities, &entityId);
}

static inline ShovelerSpatialosWorkerViewComponent *shovelerSpatialosWorkerViewEntityGetComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *component)
{
	return (ShovelerSpatialosWorkerViewComponent *) g_hash_table_lookup(entity->components, component);
}

static inline void *shovelerSpatialosWorkerViewGetTarget(ShovelerSpatialosWorkerView *view, const char *targetName)
{
	return g_hash_table_lookup(view->targets, targetName);
}

#endif
