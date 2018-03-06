#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_MODEL_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_MODEL_H

#include <stdbool.h> // bool

#include <glib.h>

#include <shoveler/drawable.h>
#include <shoveler/material.h>
#include <shoveler/model.h>
#include <shoveler/scene.h>
#include <shoveler/types.h>
#include <shoveler/spatialos/worker/view/base/view.h>

typedef enum {
	SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_CUBE,
	SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_QUAD,
	SHOVELER_SPATIALOS_WORKER_VIEW_DRAWABLE_TYPE_POINT,
} ShovelerSpatialosWorkerViewDrawableType;

typedef struct {
	ShovelerSpatialosWorkerViewDrawableType type;
} ShovelerSpatialosWorkerViewDrawableConfiguration;

typedef enum {
	SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_COLOR,
	SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_TEXTURE,
	SHOVELER_SPATIALOS_WORKER_VIEW_MATERIAL_TYPE_PARTICLE,
} ShovelerSpatialosWorkerViewMaterialType;

typedef struct {
	ShovelerSpatialosWorkerViewMaterialType type;
	ShovelerVector3 color;
	const char *texture;
} ShovelerSpatialosWorkerViewMaterialConfiguration;

typedef struct {
	ShovelerSpatialosWorkerViewDrawableConfiguration drawable;
	ShovelerSpatialosWorkerViewMaterialConfiguration material;
	ShovelerVector3 rotation;
	ShovelerVector3 scale;
	bool visible;
	bool emitter;
	bool screenspace;
	bool castsShadow;
	GLuint polygonMode;
} ShovelerSpatialosWorkerViewModelConfiguration;

typedef struct {
	ShovelerModel *model;
	ShovelerSpatialosWorkerViewComponentCallback *positionCallback;
} ShovelerSpatialosWorkerViewModel;

static const char *shovelerSpatialosWorkerViewModelComponentName = "model";

bool shovelerSpatialosWorkerViewAddEntityModel(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewModelConfiguration modelConfiguration);
bool shovelerSpatialosWorkerViewUpdateEntityModelDrawable(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewDrawableConfiguration drawableConfiguration);
bool shovelerSpatialosWorkerViewUpdateEntityModelMaterial(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewMaterialConfiguration materialConfiguration);
bool shovelerSpatialosWorkerViewUpdateEntityModelRotation(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerVector3 rotation);
bool shovelerSpatialosWorkerViewUpdateEntityModelScale(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerVector3 scale);
bool shovelerSpatialosWorkerViewUpdateEntityModelVisible(ShovelerSpatialosWorkerView *view, long long int entityId, bool visible);
bool shovelerSpatialosWorkerViewUpdateEntityModelEmitter(ShovelerSpatialosWorkerView *view, long long int entityId, bool emitter);
bool shovelerSpatialosWorkerViewUpdateEntityModelScreenspace(ShovelerSpatialosWorkerView *view, long long int entityId, bool screenspace);
bool shovelerSpatialosWorkerViewUpdateEntityModelPolygonMode(ShovelerSpatialosWorkerView *view, long long int entityId, GLuint polygonMode);
bool shovelerSpatialosWorkerViewRemoveEntityModel(ShovelerSpatialosWorkerView *view, long long int entityId);

#endif
