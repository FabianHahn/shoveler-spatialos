#include <stdlib.h> // malloc, free

#include "camera/perspective.h"
#include "types.h"

typedef struct {
	ShovelerCamera camera;
} ShovelerCameraIdentity;


static void freeIdentityCamera(void *identityCameraPointer);

ShovelerCamera *shovelerCameraIdentityCreate()
{
	ShovelerCameraIdentity *identityCamera = malloc(sizeof(ShovelerCameraIdentity));
	shovelerCameraInit(&identityCamera->camera, (ShovelerVector3){0, 0, 0}, identityCamera, freeIdentityCamera);
	return &identityCamera->camera;
}

static void freeIdentityCamera(void *identityCameraPointer)
{
	free(identityCameraPointer);
}
