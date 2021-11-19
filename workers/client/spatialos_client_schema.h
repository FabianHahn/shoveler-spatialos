#ifndef SHOVELER_CLIENT_SPATIALOS_CLIENT_SCHEMA_H
#define SHOVELER_CLIENT_SPATIALOS_CLIENT_SCHEMA_H

#include <improbable/c_schema.h>
#include <shoveler/types.h>

typedef struct ShovelerComponentStruct ShovelerComponent;
typedef struct ShovelerComponentFieldStruct ShovelerComponentField;
typedef struct ShovelerComponentFieldValueStruct ShovelerComponentFieldValue;
typedef struct ShovelerWorldStruct ShovelerWorld;

const char *shovelerClientResolveComponentTypeId(int componentId);
int shovelerClientResolveComponentSchemaId(const char *componentTypeId);
void shovelerClientApplyComponentData(ShovelerWorld *world, ShovelerComponent *component, Schema_ComponentData *componentData, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
void shovelerClientApplyComponentUpdate(ShovelerWorld *world, ShovelerComponent *component, Schema_ComponentUpdate *componentUpdate, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
Schema_ComponentUpdate *shovelerClientCreateComponentUpdate(ShovelerComponent *component, const ShovelerComponentField *field, const ShovelerComponentFieldValue *value);
Schema_ComponentUpdate *shovelerClientCreateImprobablePositionUpdate(ShovelerVector3 position, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);

#endif
