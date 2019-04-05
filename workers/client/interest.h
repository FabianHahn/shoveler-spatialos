#ifndef SHOVELER_CLIENT_INTEREST_H
#define SHOVELER_CLIENT_INTEREST_H

#include <improbable/standard_library.h>

extern "C" {
#include <shoveler/view.h>
}

improbable::ComponentInterest computeViewInterest(ShovelerView *view);

#endif
