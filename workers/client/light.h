#ifndef SHOVELER_CLIENT_LIGHT_H
#define SHOVELER_CLIENT_LIGHT_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerLightCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
