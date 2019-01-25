#ifndef SHOVELER_CLIENT_TILE_SPRITE_ANIMATION_H
#define SHOVELER_CLIENT_TILE_SPRITE_ANIMATION_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerTileSpriteAnimationCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
