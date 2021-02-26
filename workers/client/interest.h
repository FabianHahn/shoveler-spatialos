#ifndef SHOVELER_CLIENT_INTEREST_H
#define SHOVELER_CLIENT_INTEREST_H

#include <improbable/c_schema.h>
#include <shoveler/view.h>
#include <shoveler/types.h>

int shovelerClientComputeViewInterest(ShovelerView *view, long long int clientEntityId, bool useAbsoluteConstraint, ShovelerVector3 absolutePosition, double viewDistance, Schema_Object *outputComponentSetInterest);

#endif
