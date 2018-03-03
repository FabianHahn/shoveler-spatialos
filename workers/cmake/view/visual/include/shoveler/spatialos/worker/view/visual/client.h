#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_CLIENT_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_CLIENT_H

#include <shoveler/spatialos/worker/view/base/view.h>

static const char *shovelerSpatialosWorkerViewClientComponentName = "client";

bool shovelerSpatialosWorkerViewAddEntityClient(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewDelegateClient(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewUndelegateClient(ShovelerSpatialosWorkerView *view, long long int entityId);
bool shovelerSpatialosWorkerViewRemoveEntityClient(ShovelerSpatialosWorkerView *view, long long int entityId);

#endif
