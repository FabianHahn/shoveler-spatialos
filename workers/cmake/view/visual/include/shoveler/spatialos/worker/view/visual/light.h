#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_LIGHT_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_LIGHT_H

#include <shoveler/spatialos/worker/view/base/view.h>

typedef enum {
	SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_SPOT,
	SHOVELER_SPATIALOS_WORKER_VIEW_LIGHT_TYPE_POINT
} ShovelerSpatialosWorkerViewLightType;

typedef struct {
	ShovelerSpatialosWorkerViewLightType type;
	int width;
	int height;
	GLsizei samples;
	float ambientFactor;
	float exponentialFactor;
	ShovelerVector3 color;
} ShovelerSpatialosWorkerViewLightConfiguration;

static const char *shovelerSpatialosWorkerViewLightComponentName = "light";

bool shovelerSpatialosWorkerViewAddEntityLight(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewLightConfiguration lightConfiguration);
bool shovelerSpatialosWorkerViewRemoveEntityLight(ShovelerSpatialosWorkerView *view, long long int entityId);

#endif
