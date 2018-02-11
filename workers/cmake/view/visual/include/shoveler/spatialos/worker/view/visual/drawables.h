#ifndef SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_DRAWABLES_H
#define SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_DRAWABLES_H

#include <shoveler/scene.h>
#include <shoveler/spatialos/worker/view/base/view.h>

typedef struct {
	ShovelerDrawable *cube;
	ShovelerDrawable *quad;
	ShovelerDrawable *point;
} ShovelerSpatialosWorkerViewDrawables;

static const char *shovelerSpatialosWorkerViewDrawablesTargetName = "drawables";

ShovelerSpatialosWorkerViewDrawables *shovelerSpatialosWorkerViewDrawablesCreate(ShovelerSpatialosWorkerView *view);
void shovelerSpatialosWorkerViewDrawablesFree(ShovelerSpatialosWorkerViewDrawables *drawables);

static inline ShovelerSpatialosWorkerViewDrawables *shovelerSpatialosWorkerViewGetDrawables(ShovelerSpatialosWorkerView *view)
{
	return (ShovelerSpatialosWorkerViewDrawables *) shovelerSpatialosWorkerViewGetTarget(view, shovelerSpatialosWorkerViewDrawablesTargetName);
}

static inline bool shovelerSpatialosWorkerViewHasDrawables(ShovelerSpatialosWorkerView *view)
{
	return shovelerSpatialosWorkerViewGetDrawables(view) != NULL;
}

#endif
