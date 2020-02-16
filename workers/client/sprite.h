#ifndef SHOVELER_CLIENT_SPRITE_H
#define SHOVELER_CLIENT_SPRITE_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerSpriteCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
