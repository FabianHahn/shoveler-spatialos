#ifndef SHOVELER_CLIENT_CANVAS_H
#define SHOVELER_CLIENT_CANVAS_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerCanvasCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
