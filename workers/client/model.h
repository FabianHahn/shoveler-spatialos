#ifndef SHOVELER_CLIENT_MODEL_H
#define SHOVELER_CLIENT_MODEL_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerModelCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
