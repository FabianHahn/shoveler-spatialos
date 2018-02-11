#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_SCENE_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_SCENE_H

#include <shoveler/scene.h>
#include <shoveler/spatialos/worker/view/base/view.h>

static inline ShovelerScene *shovelerSpatialosWorkerViewGetScene(ShovelerSpatialosWorkerView *view)
{
	return (ShovelerScene *) shovelerSpatialosWorkerViewGetTarget(view, "scene");
}

static inline bool shovelerSpatialosWorkerViewHasScene(ShovelerSpatialosWorkerView *view)
{
	return shovelerSpatialosWorkerViewGetScene(view) != NULL;
}

#endif
