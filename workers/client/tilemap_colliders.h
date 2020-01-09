#ifndef SHOVELER_CLIENT_TILEMAP_COLLIDERS_H
#define SHOVELER_CLIENT_TILEMAP_COLLIDERS_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerTilemapCollidersCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
