#include "shoveler/component/canvas.h"

#include "shoveler/canvas.h"
#include "shoveler/colliders.h"
#include "shoveler/component/colliders.h"
#include "shoveler/component.h"
#include "shoveler/log.h"

const char *const shovelerComponentTypeIdCanvas = "canvas";

static void *activateCanvasComponent(ShovelerComponent *component);
static void deactivateCanvasComponent(ShovelerComponent *component);

ShovelerComponentType *shovelerComponentCreateCanvasType()
{
	ShovelerComponentTypeConfigurationOption configurationOptions[1];
	configurationOptions[SHOVELER_COMPONENT_CANVAS_OPTION_ID_NUM_LAYERS] = shovelerComponentTypeConfigurationOption("num_layers", SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_INT, /* isOptional */ false, /* liveUpdate */ NULL);

	return shovelerComponentTypeCreate(shovelerComponentTypeIdCanvas, activateCanvasComponent, /* update */ NULL, deactivateCanvasComponent, /* requiresAuthority */ false, sizeof(configurationOptions) / sizeof(configurationOptions[0]), configurationOptions);
}

ShovelerCanvas *shovelerComponentGetCanvas(ShovelerComponent *component)
{
	assert(component->type->id == shovelerComponentTypeIdCanvas);

	return component->data;
}

static void *activateCanvasComponent(ShovelerComponent *component)
{
	assert(shovelerComponentHasViewColliders(component));

	int numLayers = shovelerComponentGetConfigurationValueInt(component, SHOVELER_COMPONENT_CANVAS_OPTION_ID_NUM_LAYERS);
	if(numLayers < 1) {
		shovelerLogWarning("Failed to activate canvas component on entity %lld: num_layers option must be positive, but got %d.", component->entityId, numLayers);
		return NULL;
	}

	ShovelerCanvas *canvas = shovelerCanvasCreate(numLayers);

	ShovelerColliders *colliders = shovelerComponentGetViewColliders(component);
	shovelerCollidersAddCollider2(colliders, &canvas->collider);

	return canvas;
}

static void deactivateCanvasComponent(ShovelerComponent *component)
{
	ShovelerCanvas *canvas = (ShovelerCanvas *) component->data;

	ShovelerColliders *colliders = shovelerComponentGetViewColliders(component);
	shovelerCollidersRemoveCollider2(colliders, &canvas->collider);

	shovelerCanvasFree(component->data);
}
