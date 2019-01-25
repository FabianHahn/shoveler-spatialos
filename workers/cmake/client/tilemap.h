#ifndef SHOVELER_CLIENT_TILEMAP_H
#define SHOVELER_CLIENT_TILEMAP_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerTilemapCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
