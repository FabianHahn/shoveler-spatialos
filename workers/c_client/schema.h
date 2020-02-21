#ifndef SHOVELER_CLIENT_SCHEMA_H
#define SHOVELER_CLIENT_SCHEMA_H

#include <improbable/c_schema.h>

typedef struct ShovelerComponentStruct ShovelerComponent;
typedef struct ShovelerViewStruct ShovelerView;

const char *shovelerClientResolveComponentTypeId(int componentId);
int shovelerClientResolveComponentSchemaId(const char *componentTypeId);
void shovelerClientRegisterViewComponentTypes(ShovelerView *view);
void shovelerClientApplyComponentData(ShovelerView *view, ShovelerComponent *component, Schema_ComponentData *componentData, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
void shovelerClientApplyComponentUpdate(ShovelerView *view, ShovelerComponent *component, Schema_ComponentUpdate *componentUpdate, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
Schema_ComponentUpdate *shovelerClientCreateComponentUpdate(ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);

#endif
