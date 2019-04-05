#ifndef SHOVELER_CLIENT_TILE_SPRITE_H
#define SHOVELER_CLIENT_TILE_SPRITE_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerTileSpriteCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
