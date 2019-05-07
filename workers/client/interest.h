#ifndef SHOVELER_CLIENT_INTEREST_H
#define SHOVELER_CLIENT_INTEREST_H

#include <improbable/standard_library.h>

extern "C" {
#include <shoveler/view.h>
#include <shoveler/types.h>
}

improbable::ComponentInterest computeViewInterest(ShovelerView *view, bool useAbsoluteConstraint, ShovelerVector3 absolutePosition, double edgeLength);

#endif
