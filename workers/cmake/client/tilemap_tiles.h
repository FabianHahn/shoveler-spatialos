#ifndef SHOVELER_CLIENT_TILEMAP_TILES_H
#define SHOVELER_CLIENT_TILEMAP_TILES_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerTilemapTilesCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
