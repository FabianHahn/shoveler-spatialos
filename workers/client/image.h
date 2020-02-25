#ifndef SHOVELER_CLIENT_IMAGE_H
#define SHOVELER_CLIENT_IMAGE_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerImageCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
