#include <stdlib.h> // malloc free
#include <string.h> // strdup

#include "shoveler/spatialos/worker/view/base/view.h"

static void freeEntity(void *entityPointer);
static void freeComponent(void *componentPointer);
static void freeCallbacks(void *callbacksPointer);

ShovelerSpatialosWorkerView *shovelerSpatialosWorkerViewCreate()
{
	ShovelerSpatialosWorkerView *view = malloc(sizeof(ShovelerSpatialosWorkerView));
	view->entities = g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, freeEntity);
	view->targets = g_hash_table_new_full(&g_str_hash, &g_str_equal, &free, NULL);

	return view;
}

bool shovelerSpatialosWorkerViewAddEntity(ShovelerSpatialosWorkerView *view, long long int entityId)
{
	ShovelerSpatialosWorkerViewEntity *entity = malloc(sizeof(ShovelerSpatialosWorkerViewEntity));
	entity->view = view;
	entity->entityId = entityId;
	entity->components = g_hash_table_new_full(&g_str_hash, &g_str_equal, NULL, &freeComponent);
	entity->callbacks = g_hash_table_new_full(&g_str_hash, &g_str_equal, &free, &freeCallbacks);

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

bool shovelerSpatialosWorkerViewEntityAddComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, void *data, ShovelerSpatialosWorkerViewComponentFreeFunction *freeFunction)
{
	ShovelerSpatialosWorkerViewComponent *component = malloc(sizeof(ShovelerSpatialosWorkerViewComponentFreeFunction));
	component->entity = entity;
	component->name = strdup(componentName);
	component->data = data;
	component->free = freeFunction;

	if(!g_hash_table_insert(entity->components, component->name, component)) {
		freeComponent(component);
		return false;
	}

	GQueue *callbacks = g_hash_table_lookup(entity->callbacks, componentName);
	if(callbacks != NULL) {
		for(GList *iter = callbacks->head; iter != NULL; iter = iter->next) {
			ShovelerSpatialosWorkerViewComponentCallback *callback = iter->data;
			callback->function(component, VIEW_COMPONENT_CALLBACK_ADD, callback->userData);
		}
	}

	return true;
}

bool shovelerSpatialosWorkerViewEntityUpdateComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName)
{
	ShovelerSpatialosWorkerViewComponent *component = g_hash_table_lookup(entity->components, componentName);
	if(component == NULL) {
		return false;
	}

	GQueue *callbacks = g_hash_table_lookup(entity->callbacks, componentName);
	if(callbacks != NULL) {
		for(GList *iter = callbacks->head; iter != NULL; iter = iter->next) {
			ShovelerSpatialosWorkerViewComponentCallback *callback = iter->data;
			callback->function(component, VIEW_COMPONENT_CALLBACK_UPDATE, callback->userData);
		}
	}

	return true;
}

bool shovelerSpatialosWorkerViewEntityRemoveComponent(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName)
{
	ShovelerSpatialosWorkerViewComponent *component = g_hash_table_lookup(entity->components, componentName);
	if(component == NULL) {
		return false;
	}

	GQueue *callbacks = g_hash_table_lookup(entity->callbacks, componentName);
	if(callbacks != NULL) {
		for(GList *iter = callbacks->head; iter != NULL; iter = iter->next) {
			ShovelerSpatialosWorkerViewComponentCallback *callback = iter->data;
			callback->function(component, VIEW_COMPONENT_CALLBACK_REMOVE, callback->userData);
		}
	}

	return g_hash_table_remove(entity->components, componentName);
}

ShovelerSpatialosWorkerViewComponentCallback *shovelerSpatialosWorkerViewEntityAddCallback(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, ShovelerSpatialosWorkerViewComponentCallbackFunction *function, void *userData)
{
	GQueue *callbacks = g_hash_table_lookup(entity->callbacks, componentName);
	if(callbacks == NULL) {
		callbacks = g_queue_new();
		g_hash_table_insert(entity->callbacks, strdup(componentName), callbacks);
	}

	ShovelerSpatialosWorkerViewComponentCallback *callback = malloc(sizeof(ShovelerSpatialosWorkerViewComponentCallback));
	callback->function = function;
	callback->userData = userData;
	g_queue_push_tail(callbacks, callback);

	// if the target component is already there, trigger the add callback directly
	ShovelerSpatialosWorkerViewComponent *component = g_hash_table_lookup(entity->components, componentName);
	if(component != NULL) {
		callback->function(component, VIEW_COMPONENT_CALLBACK_ADD, callback->userData);
	}

	return callback;
}

bool shovelerSpatialosWorkerViewSetTarget(ShovelerSpatialosWorkerView *view, const char *targetName, void *target)
{
	return g_hash_table_insert(view->targets, strdup(targetName), target);
}

bool shovelerSpatialosWorkerViewEntityRemoveCallback(ShovelerSpatialosWorkerViewEntity *entity, const char *componentName, ShovelerSpatialosWorkerViewComponentCallback *callback)
{
	GQueue *callbacks = g_hash_table_lookup(entity->callbacks, componentName);
	if(callbacks == NULL) {
		return false;
	}

	if(!g_queue_remove(callbacks, callback)) {
		return false;
	}

	free(callback);
	return true;
}

void shovelerSpatialosWorkerViewFree(ShovelerSpatialosWorkerView *view)
{
	g_hash_table_destroy(view->entities);
	g_hash_table_destroy(view->targets);
	free(view);
}

static void freeEntity(void *entityPointer)
{
	ShovelerSpatialosWorkerViewEntity *entity = entityPointer;
	g_hash_table_destroy(entity->components);
	g_hash_table_destroy(entity->callbacks);
	free(entity);
}

static void freeComponent(void *componentPointer)
{
	ShovelerSpatialosWorkerViewComponent *component = componentPointer;
	component->free(component);
	free(component->name);
	free(component);
}

static void freeCallbacks(void *callbacksPointer)
{
	GQueue *callbacks = callbacksPointer;
	g_queue_free(callbacks);
}
