#ifndef SHOVELER_CLIENT_MATERIAL_H
#define SHOVELER_CLIENT_MATERIAL_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerMaterialCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
