#ifndef SHOVELER_CLIENT_TILESET_H
#define SHOVELER_CLIENT_TILESET_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerTilesetCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
