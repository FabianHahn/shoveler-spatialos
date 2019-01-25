#ifndef SHOVELER_CLIENT_CHUNK_H
#define SHOVELER_CLIENT_CHUNK_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerChunkCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
