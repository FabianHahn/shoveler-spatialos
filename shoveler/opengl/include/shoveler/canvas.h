#ifndef SHOVELER_CANVAS_H
#define SHOVELER_CANVAS_H

#include <glib.h>
#include <shoveler/collider.h>
#include <shoveler/types.h>
#include <stdbool.h> // bool

typedef struct ShovelerCameraStruct ShovelerCamera; // forward declaration: camera.h
typedef struct ShovelerFontAtlasTextureStruct
    ShovelerFontAtlasTexture; // forward declaration: font_atlas_texture.h
typedef struct ShovelerLightStruct ShovelerLight; // forward declaration: light.h
typedef struct ShovelerMaterialStruct ShovelerMaterial; // forward declaration: material.h
typedef struct ShovelerModelStruct ShovelerModel; // forward declaration: model.h
typedef struct ShovelerRenderStateStruct ShovelerRenderState; // forward declaration: render_state.h
typedef struct ShovelerSceneStruct ShovelerScene; // forward declaration: scene.h
typedef struct ShovelerSpriteStruct ShovelerSprite; // forward declaration: sprite.h

typedef struct ShovelerCanvasStruct {
  ShovelerCollider2 collider;
  int numLayers;
  /** array of size numLayers, wher each element is a list of (ShovelerSprite *) */
  GQueue** layers;
} ShovelerCanvas;

ShovelerCanvas* shovelerCanvasCreate(int numLayers);
/** Adds a sprite to the canvas, with the caller retaining ownership over it and changes to it being
 * reflected live. Returns the id of the added sprite. */
void shovelerCanvasAddSprite(ShovelerCanvas* canvas, int layerId, ShovelerSprite* sprite);
/** Removes a sprite from a given layer of the canvas. */
bool shovelerCanvasRemoveSprite(ShovelerCanvas* canvas, int layerId, ShovelerSprite* sprite);
bool shovelerCanvasRender(
    ShovelerCanvas* canvas,
    ShovelerVector2 regionPosition,
    ShovelerVector2 regionSize,
    ShovelerScene* scene,
    ShovelerCamera* camera,
    ShovelerLight* light,
    ShovelerModel* model,
    ShovelerRenderState* renderState);
void shovelerCanvasFree(ShovelerCanvas* canvas);

#endif
