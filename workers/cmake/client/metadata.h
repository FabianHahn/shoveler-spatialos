#ifndef SHOVELER_CLIENT_METADATA_H
#define SHOVELER_CLIENT_METADATA_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerMetadataCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
