#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_BASE_POSITION_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_BASE_POSITION_H

#include <stdbool.h> // bool

#include <glib.h>

#include <shoveler/spatialos/worker/view/base/view.h>

typedef struct {
	double x;
	double y;
	double z;
} ShovelerSpatialosWorkerViewPosition;

static const char *shovelerSpatialosWorkerViewPositionComponentName = "position";

bool shovelerSpatialosWorkerViewAddEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z);
bool shovelerSpatialosWorkerViewUpdateEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId, double x, double y, double z);
bool shovelerSpatialosWorkerViewRemoveEntityPosition(ShovelerSpatialosWorkerView *view, long long int entityId);

#endif
