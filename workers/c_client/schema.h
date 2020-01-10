#ifndef SHOVELER_CLIENT_SCHEMA_H
#define SHOVELER_CLIENT_SCHEMA_H

#include <improbable/c_schema.h>

typedef struct ShovelerComponentStruct ShovelerComponent;
typedef struct ShovelerViewStruct ShovelerView;

const char *shovelerClientResolveComponentTypeId(int componentId);
void shovelerClientRegisterViewComponentTypes(ShovelerView *view);
void shovelerClientApplyComponentData(ShovelerView *view, ShovelerComponent *component, Schema_ComponentData *componentData);

#endif
