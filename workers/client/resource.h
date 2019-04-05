#ifndef SHOVELER_CLIENT_RESOURCE_H
#define SHOVELER_CLIENT_RESOURCE_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerResourceCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
