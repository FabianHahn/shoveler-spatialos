#ifndef SHOVELER_CLIENT_DRAWABLE_H
#define SHOVELER_CLIENT_DRAWABLE_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerDrawableCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
