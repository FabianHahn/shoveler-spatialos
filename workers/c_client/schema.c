#include <limits.h> // INT_MAX
#include <stddef.h>
#include <stdlib.h>

#include <shoveler/component/canvas.h>
#include <shoveler/component/chunk.h>
#include <shoveler/component/chunk_layer.h>
#include <shoveler/component/client.h>
#include <shoveler/component/drawable.h>
#include <shoveler/component/light.h>
#include <shoveler/component/material.h>
#include <shoveler/component/model.h>
#include <shoveler/component/position.h>
#include <shoveler/component/resource.h>
#include <shoveler/component/sampler.h>
#include <shoveler/component/texture.h>
#include <shoveler/component/tile_sprite.h>
#include <shoveler/component/tile_sprite_animation.h>
#include <shoveler/component/tilemap.h>
#include <shoveler/component/tilemap_colliders.h>
#include <shoveler/component/tilemap_tiles.h>
#include <shoveler/component/tileset.h>
#include <shoveler/component.h>
#include <shoveler/log.h>
#include <shoveler/view.h>

#include "schema.h"

static void updatePositionCoordinates(ShovelerComponent *component, Schema_Object *fields, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ);
static void updateComponentConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, Schema_FieldId fieldId, bool clear_if_not_set);
static void updateEntityIdConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateEntityIdArrayConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId);
static void updateFloatConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateBoolConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateIntConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateStringConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateVector2ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateVector3ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateVector4ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);
static void updateBytesConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set);

const char *shovelerClientResolveComponentTypeId(int componentId)
{
	switch(componentId) {
		case 54:
			return shovelerComponentTypeIdPosition;
		case 1335:
			return shovelerComponentTypeIdClient;
		case 1337:
			return shovelerComponentTypeIdResource;
		case 1338:
			return shovelerComponentTypeIdTexture;
		case 1339:
			return shovelerComponentTypeIdTileset;
		case 1340:
			return shovelerComponentTypeIdTilemapTiles;
		case 1341:
			return shovelerComponentTypeIdTilemap;
		case 1342:
			return shovelerComponentTypeIdTileSprite;
		case 1343:
			return shovelerComponentTypeIdTileSpriteAnimation;
		case 1344:
			return shovelerComponentTypeIdCanvas;
		case 1345:
			return shovelerComponentTypeIdChunkLayer;
		case 1346:
			return shovelerComponentTypeIdDrawable;
		case 1347:
			return shovelerComponentTypeIdMaterial;
		case 1348:
			return shovelerComponentTypeIdModel;
		case 1349:
			return shovelerComponentTypeIdLight;
		case 13381:
			return shovelerComponentTypeIdSampler;
		case 134132:
			return shovelerComponentTypeIdTilemapColliders;
		case 13451337:
			return shovelerComponentTypeIdChunk;
		default:
			return NULL;
	}
}

void shovelerClientRegisterViewComponentTypes(ShovelerView *view)
{
	shovelerViewAddComponentType(view, shovelerComponentCreateCanvasType());
	shovelerViewAddComponentType(view, shovelerComponentCreateChunkType());
	shovelerViewAddComponentType(view, shovelerComponentCreateChunkLayerType());
	shovelerViewAddComponentType(view, shovelerComponentCreateClientType());
	shovelerViewAddComponentType(view, shovelerComponentCreateDrawableType());
	shovelerViewAddComponentType(view, shovelerComponentCreateLightType());
	shovelerViewAddComponentType(view, shovelerComponentCreateMaterialType());
	shovelerViewAddComponentType(view, shovelerComponentCreateModelType());
	shovelerViewAddComponentType(view, shovelerComponentCreatePositionType());
	shovelerViewAddComponentType(view, shovelerComponentCreateResourceType());
	shovelerViewAddComponentType(view, shovelerComponentCreateSamplerType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTextureType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTileSpriteType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTileSpriteAnimationType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapColliders());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapTilesType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilesetType());
}

void shovelerClientApplyComponentData(ShovelerView *view, ShovelerComponent *component, Schema_ComponentData *componentData, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_Object *fields = Schema_GetComponentDataFields(componentData);

	// special case position updates
	if (component->type->id == shovelerComponentTypeIdPosition) {
		updatePositionCoordinates(component, fields, mappingX, mappingY, mappingZ);
		return;
	}

	for(int optionId = 0; optionId < component->type->numConfigurationOptions; optionId++) {
		Schema_FieldId fieldId = optionId + 1;
		ShovelerComponentTypeConfigurationOption *configurationOption = &component->type->configurationOptions[optionId];

		updateComponentConfigurationOption(component, configurationOption, optionId, fields, fieldId, configurationOption->isOptional);
	}
}

void shovelerClientApplyComponentUpdate(ShovelerView *view, ShovelerComponent *component, Schema_ComponentUpdate *componentUpdate, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_Object *fields = Schema_GetComponentUpdateFields(componentUpdate);

	// special case position updates
	if(component->type->id == shovelerComponentTypeIdPosition) {
		updatePositionCoordinates(component, fields, mappingX, mappingY, mappingZ);
		return;
	}

	uint32_t cleared_field_count = Schema_GetComponentUpdateClearedFieldCount(componentUpdate);
	assert(cleared_field_count <= INT_MAX);
	for(int i = 0; i < (int) cleared_field_count; i++) {
		Schema_FieldId fieldId = Schema_IndexComponentUpdateClearedField(componentUpdate, i);
		assert(fieldId < INT_MAX);
		int optionId = (int) fieldId - 1;
		ShovelerComponentTypeConfigurationOption *configurationOption = &component->type->configurationOptions[optionId];

		if(configurationOption->type != SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_ENTITY_ID_ARRAY && !configurationOption->isOptional) {
			shovelerLogWarning("Received clear for non-optional entity %lld component '%s' option '%s', ignoring.", component->entityId, component->type->id, configurationOption->name);
			continue;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	}

	for(int optionId = 0; optionId < component->type->numConfigurationOptions; optionId++) {
		Schema_FieldId fieldId = optionId + 1;
		ShovelerComponentTypeConfigurationOption *configurationOption = &component->type->configurationOptions[optionId];

		updateComponentConfigurationOption(component, configurationOption, optionId, fields, fieldId, /* clear_if_not_set */ false);
	}
}

static void updatePositionCoordinates(ShovelerComponent *component, Schema_Object *fields, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	if(Schema_GetObjectCount(fields, /* fieldId */ 1) != 1) {
		shovelerLogWarning("Received entity %lld component 'position' update without coordinates value, ignoring.", component->entityId);
		return;
	}

	Schema_Object *coordinatesField = Schema_GetObject(fields, /* fieldId */ 1);
	double coordinatesX = Schema_GetDouble(coordinatesField, /* fieldId */ 1);
	double coordinatesY = Schema_GetDouble(coordinatesField, /* fieldId */ 2);
	double coordinatesZ = Schema_GetDouble(coordinatesField, /* fieldId */ 3);
	ShovelerVector3 coordinates = shovelerVector3((float) coordinatesX, (float) coordinatesY, (float) coordinatesZ);

	float mappedCoordinatesX = shovelerCoordinateMap(coordinates, mappingX);
	float mappedCoordinatesY = shovelerCoordinateMap(coordinates, mappingY);
	float mappedCoordinatesZ = shovelerCoordinateMap(coordinates, mappingZ);
	ShovelerVector3 mappedCoordinates = shovelerVector3(mappedCoordinatesX, mappedCoordinatesY, mappedCoordinatesZ);

	shovelerComponentUpdateCanonicalConfigurationOptionVector3(component, SHOVELER_COMPONENT_POSITION_OPTION_ID_COORDINATES, mappedCoordinates);

	shovelerLogInfo("Updated entity %lld component 'position' to mapped coordinates value (%f, %f, %f).", component->entityId, mappedCoordinatesX, mappedCoordinatesY, mappedCoordinatesZ);
}

static void updateComponentConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, Schema_FieldId fieldId, bool clear_if_not_set)
{
	switch(configurationOption->type) {
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_ENTITY_ID: {
			updateEntityIdConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_ENTITY_ID_ARRAY: {
			updateEntityIdArrayConfigurationOption(component, configurationOption, optionId, fields, fieldId);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_FLOAT: {
			updateFloatConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_BOOL: {
			updateBoolConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_INT: {
			updateIntConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_STRING: {
			updateStringConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR2: {
			updateVector2ConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR3: {
			updateVector3ConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR4: {
			updateVector4ConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_BYTES: {
			updateBytesConfigurationOption(component, configurationOption, optionId, fields, fieldId, clear_if_not_set);
		} break;
	}
}

static void updateEntityIdConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetEntityIdCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		long long int entityIdValue = Schema_GetEntityId(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionEntityId(component, optionId, entityIdValue);

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to entity ID value %lld.", component->entityId, component->type->id, configurationOption->name, entityIdValue);
	}
}

static void updateEntityIdArrayConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId)
{
	int numEntityIds = Schema_GetEntityIdCount(fields, fieldId);

	long long int *entityIdArrayValue = malloc(numEntityIds * sizeof(long long int));
	for(int j = 0; j < numEntityIds; j++) {
		entityIdArrayValue[j] = Schema_IndexEntityId(fields, fieldId, j);
	}

	shovelerComponentUpdateCanonicalConfigurationOptionEntityIdArray(component, optionId, entityIdArrayValue, numEntityIds);

	free(entityIdArrayValue);

	shovelerLogInfo("Updated entity %lld component '%s' option '%s' to entity ID list value with %d element(s).", component->entityId, component->type->id, configurationOption->name, numEntityIds);
}

static void updateFloatConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetFloatCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		float floatValue = Schema_GetFloat(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionFloat(component, optionId, floatValue);

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to float value %f.", component->entityId, component->type->id, configurationOption->name, floatValue);
	}
}

static void updateBoolConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetBoolCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		bool boolValue = Schema_GetBool(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionBool(component, optionId, boolValue);

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to bool value %s.", component->entityId, component->type->id, configurationOption->name, boolValue ? "true" : "false");
	}
}

static void updateIntConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetInt32Count(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		int intValue = Schema_GetInt32(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionInt(component, optionId, intValue);

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to int value %d.", component->entityId, component->type->id, configurationOption->name, intValue);
	}
}

static void updateStringConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetBytesCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		const char *stringValue = (const char *) Schema_GetBytes(fields, fieldId);

		shovelerComponentUpdateCanonicalConfigurationOptionString(component, optionId, stringValue);

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to string value '%s'.", component->entityId, component->type->id, configurationOption->name, stringValue);
	}
}

static void updateVector2ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetObjectCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		Schema_Object *vector2 = Schema_GetObject(fields, fieldId);
		float x = Schema_GetFloat(vector2, 1);
		float y = Schema_GetFloat(vector2, 2);

		shovelerComponentUpdateCanonicalConfigurationOptionVector2(component, optionId, shovelerVector2(x, y));

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to vector2 value (%f, %f).", component->entityId, component->type->id, configurationOption->name, x, y);
	}
}

static void updateVector3ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetObjectCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		Schema_Object *vector3 = Schema_GetObject(fields, fieldId);
		float x = Schema_GetFloat(vector3, 1);
		float y = Schema_GetFloat(vector3, 2);
		float z = Schema_GetFloat(vector3, 3);

		shovelerComponentUpdateCanonicalConfigurationOptionVector3(component, optionId, shovelerVector3(x, y, z));

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to vector3 value (%f, %f, %f).", component->entityId, component->type->id, configurationOption->name, x, y, z);
	}
}

static void updateVector4ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetObjectCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		Schema_Object *vector4 = Schema_GetObject(fields, fieldId);
		float x = Schema_GetFloat(vector4, 1);
		float y = Schema_GetFloat(vector4, 2);
		float z = Schema_GetFloat(vector4, 3);
		float w = Schema_GetFloat(vector4, 4);

		shovelerComponentUpdateCanonicalConfigurationOptionVector4(component, optionId, shovelerVector4(x, y, z, w));

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to vector4 value (%f, %f, %f, %f).", component->entityId, component->type->id, configurationOption->name, x, y, z, w);
	}
}

static void updateBytesConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetBytesCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogInfo("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		int bytesLength = (int) Schema_GetBytesLength(fields, fieldId);
		const unsigned char *bytesValue = Schema_GetBytes(fields, fieldId);

		shovelerComponentUpdateCanonicalConfigurationOptionBytes(component, optionId, bytesValue, bytesLength);

		shovelerLogInfo("Updated entity %lld component '%s' option '%s' to %d bytes value.", component->entityId, component->type->id, configurationOption->name, bytesLength);
	}
}
