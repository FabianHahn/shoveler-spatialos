#ifndef SHOVELER_CLIENT_TEXTURE_H
#define SHOVELER_CLIENT_TEXTURE_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerTextureCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
