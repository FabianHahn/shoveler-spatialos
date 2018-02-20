#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_BASE_POSITION_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_BASE_POSITION_H

#include <stdbool.h> // bool

#include <glib.h>

#include <shoveler/spatialos/worker/view/base/view.h>

typedef void (ShovelerSpatialosWorkerViewPositionRequestUpdateFunction)(ShovelerSpatialosWorkerViewComponent *component, double x, double y, double z, void *userData);

typedef struct {
	double x;
	double y;
	double z;
	ShovelerSpatialosWorkerViewPositionRequestUpdateFunction *requestUpdate;
	void *requestUpdateUserData;
} ShovelerSpatialosWorkerViewPosition;

static const char *shovelerSpatialosWorkerViewPositionComponentName = "position";

bool shovelerSpatialosWorkerViewAddEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z);
bool shovelerSpatialosWorkerViewUpdateEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z);
bool shovelerSpatialosWorkerViewDelegatePosition(ShovelerSpatialosWorkerView *view, long long int entityId, ShovelerSpatialosWorkerViewPositionRequestUpdateFunction *requestUpdateFunction, void *userData);
bool shovelerSpatialosWorkerViewUndelegatePosition(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewRequestPositionUpdate(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z);
bool shovelerSpatialosWorkerViewRemoveEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId);

#endif
