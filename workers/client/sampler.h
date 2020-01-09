#ifndef SHOVELER_CLIENT_SAMPLER_H
#define SHOVELER_CLIENT_SAMPLER_H

#include <improbable/worker.h>

extern "C" {
#include <shoveler/view.h>
}

void registerSamplerCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view);

#endif
