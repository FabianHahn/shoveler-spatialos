#include <stddef.h>

#include <shoveler/component/client.h>
#include <shoveler/component/position.h>

#include "schema.h"

const char *shovelerClientResolveComponentTypeId(int componentId)
{
	switch(componentId) {
		case 54:
			return shovelerComponentTypeIdPosition;
		case 1335:
			return shovelerComponentTypeIdClient;
		default:
			return NULL;
	}
}
