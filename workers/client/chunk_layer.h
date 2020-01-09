#ifndef SHOVELER_CLIENT_CHUNK_LAYER_H
#define SHOVELER_CLIENT_CHUNK_LAYER_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerChunkLayerCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
