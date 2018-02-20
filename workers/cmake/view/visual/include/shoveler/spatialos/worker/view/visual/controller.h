#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_CONTROLLER_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_CONTROLLER_H

#include <shoveler/controller.h>
#include <shoveler/spatialos/worker/view/base/view.h>

static inline ShovelerController *shovelerSpatialosWorkerViewGetController(ShovelerSpatialosWorkerView *view)
{
	return (ShovelerController *) shovelerSpatialosWorkerViewGetTarget(view, "controller");
}

static inline bool shovelerSpatialosWorkerViewHasController(ShovelerSpatialosWorkerView *view)
{
	return shovelerSpatialosWorkerViewGetController(view) != NULL;
}

#endif
