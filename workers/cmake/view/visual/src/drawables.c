#include <assert.h> // assert
#include <stdlib.h> // malloc free

#include <shoveler/drawable/cube.h>
#include <shoveler/drawable/quad.h>
#include <shoveler/drawable/point.h>

#include "shoveler/spatialos/worker/view/visual/drawables.h"

ShovelerSpatialosWorkerViewDrawables *shovelerSpatialosWorkerViewDrawablesCreate(ShovelerSpatialosWorkerView *view)
{
	ShovelerSpatialosWorkerViewDrawables *drawables = malloc(sizeof(ShovelerSpatialosWorkerViewDrawables));
	drawables->cube = shovelerDrawableCubeCreate();
	drawables->quad = shovelerDrawableQuadCreate();
	drawables->point = shovelerDrawablePointCreate();

	shovelerSpatialosWorkerViewSetTarget(view, shovelerSpatialosWorkerViewDrawablesTargetName, drawables);

	return drawables;
}

void shovelerSpatialosWorkerViewDrawablesFree(ShovelerSpatialosWorkerViewDrawables *drawables)
{
	shovelerDrawableFree(drawables->cube);
	shovelerDrawableFree(drawables->quad);
	shovelerDrawableFree(drawables->point);
	free(drawables);
}
