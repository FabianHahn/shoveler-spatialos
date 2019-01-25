#ifndef SHOVELER_CLIENT_COORDINATE_MAPPING_H
#define SHOVELER_CLIENT_COORDINATE_MAPPING_H

#include <shoveler.h>

extern "C" {
#include <shoveler/types.h>
}

static inline ShovelerCoordinateMapping convertCoordinateMapping(shoveler::CoordinateMapping mapping)
{
	switch(mapping) {
		case shoveler::CoordinateMapping::POSITIVE_X:
			return SHOVELER_COORDINATE_MAPPING_POSITIVE_X;
		case shoveler::CoordinateMapping::NEGATIVE_X:
			return SHOVELER_COORDINATE_MAPPING_NEGATIVE_X;
		case shoveler::CoordinateMapping::POSITIVE_Y:
			return SHOVELER_COORDINATE_MAPPING_POSITIVE_Y;
		case shoveler::CoordinateMapping::NEGATIVE_Y:
			return SHOVELER_COORDINATE_MAPPING_NEGATIVE_Y;
		case shoveler::CoordinateMapping::POSITIVE_Z:
			return SHOVELER_COORDINATE_MAPPING_POSITIVE_Z;
		case shoveler::CoordinateMapping::NEGATIVE_Z:
			return SHOVELER_COORDINATE_MAPPING_NEGATIVE_Z;
	}
}

#endif
