#include "shoveler/schema.h"

#include <stdlib.h> // NULL

const char *shovelerWorkerSchemaResolveSpecialComponentId(int componentId)
{
	switch(componentId) {
		case 50:
			return "EntityAcl";
		case 53:
			return "Metadata";
		case 54:
			return "Position";
		case 55:
			return "Persistence";
		case 58:
			return "Interest";
		case 59:
			return "System";
		case 60:
			return "Worker";
		case 61:
			return "PlayerClient";
			// special shoveler components below
		case 1334:
			return "Bootstrap";
		case 133742:
			return "ClientInfo";
		case 13351:
			return "ClientHeartbeatPing";
		case 13352:
			return "ClientHeartbeatPong";
		default:
			return NULL;
	}
}
