#include <limits.h> // INT_MAX
#include <stddef.h>
#include <stdlib.h>
#include <string.h> // strlen memcpy

#include <shoveler/component/canvas.h>
#include <shoveler/component/client.h>
#include <shoveler/component/drawable.h>
#include <shoveler/component/font.h>
#include <shoveler/component/font_atlas.h>
#include <shoveler/component/font_atlas_texture.h>
#include <shoveler/component/image.h>
#include <shoveler/component/light.h>
#include <shoveler/component/material.h>
#include <shoveler/component/model.h>
#include <shoveler/component/position.h>
#include <shoveler/component/resource.h>
#include <shoveler/component/sampler.h>
#include <shoveler/component/sprite.h>
#include <shoveler/component/texture_sprite.h>
#include <shoveler/component/text_texture_renderer.h>
#include <shoveler/component/texture.h>
#include <shoveler/component/tile_sprite.h>
#include <shoveler/component/tile_sprite_animation.h>
#include <shoveler/component/tilemap.h>
#include <shoveler/component/tilemap_colliders.h>
#include <shoveler/component/tilemap_sprite.h>
#include <shoveler/component/tilemap_tiles.h>
#include <shoveler/component/tileset.h>
#include <shoveler/component.h>
#include <shoveler/log.h>
#include <shoveler/schema.h>
#include <shoveler/view.h>

#include "schema.h"

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
		case shovelerWorkerSchemaComponentIdPosition:
			return shovelerComponentTypeIdPosition;
		case shovelerWorkerSchemaComponentIdClient:
			return shovelerComponentTypeIdClient;
		case shovelerWorkerSchemaComponentIdResource:
			return shovelerComponentTypeIdResource;
		case shovelerWorkerSchemaComponentIdImage:
			return shovelerComponentTypeIdImage;
		case shovelerWorkerSchemaComponentIdTexture:
			return shovelerComponentTypeIdTexture;
		case shovelerWorkerSchemaComponentIdTileset:
			return shovelerComponentTypeIdTileset;
		case shovelerWorkerSchemaComponentIdTilemapTiles:
			return shovelerComponentTypeIdTilemapTiles;
		case shovelerWorkerSchemaComponentIdTilemap:
			return shovelerComponentTypeIdTilemap;
		case shovelerWorkerSchemaComponentIdSprite:
			return shovelerComponentTypeIdSprite;
		case shovelerWorkerSchemaComponentIdTileSprite:
			return shovelerComponentTypeIdTileSprite;
		case shovelerWorkerSchemaComponentIdTilemapSprite:
			return shovelerComponentTypeIdTilemapSprite;
		case shovelerWorkerSchemaComponentIdTextureSprite:
			return shovelerComponentTypeIdTextureSprite;
		case shovelerWorkerSchemaComponentIdTileSpriteAnimation:
			return shovelerComponentTypeIdTileSpriteAnimation;
		case shovelerWorkerSchemaComponentIdCanvas:
			return shovelerComponentTypeIdCanvas;
		case shovelerWorkerSchemaComponentIdDrawable:
			return shovelerComponentTypeIdDrawable;
		case shovelerWorkerSchemaComponentIdMaterial:
			return shovelerComponentTypeIdMaterial;
		case shovelerWorkerSchemaComponentIdModel:
			return shovelerComponentTypeIdModel;
		case shovelerWorkerSchemaComponentIdLight:
			return shovelerComponentTypeIdLight;
		case shovelerWorkerSchemaComponentIdSampler:
			return shovelerComponentTypeIdSampler;
		case shovelerWorkerSchemaComponentIdTilemapColliders:
			return shovelerComponentTypeIdTilemapColliders;
		case shovelerWorkerSchemaComponentIdFont:
			return shovelerComponentTypeIdFont;
		case shovelerWorkerSchemaComponentIdFontAtlas:
			return shovelerComponentTypeIdFontAtlas;
		case shovelerWorkerSchemaComponentIdFontAtlasTexture:
			return shovelerComponentTypeIdFontAtlasTexture;
		case shovelerWorkerSchemaComponentIdTextTextureRenderer:
			return shovelerComponentTypeIdTextTextureRenderer;
		default:
			return NULL;
	}
}

int shovelerClientResolveComponentSchemaId(const char *componentTypeId)
{
	if(componentTypeId == shovelerComponentTypeIdPosition) {
		return shovelerWorkerSchemaComponentIdPosition;
	}
	if(componentTypeId == shovelerComponentTypeIdClient) {
		return shovelerWorkerSchemaComponentIdClient;
	}
	if(componentTypeId == shovelerComponentTypeIdResource) {
		return shovelerWorkerSchemaComponentIdResource;
	}
	if(componentTypeId == shovelerComponentTypeIdImage) {
		return shovelerWorkerSchemaComponentIdImage;
	}
	if(componentTypeId == shovelerComponentTypeIdTexture) {
		return shovelerWorkerSchemaComponentIdTexture;
	}
	if(componentTypeId == shovelerComponentTypeIdTileset) {
		return shovelerWorkerSchemaComponentIdTileset;
	}
	if(componentTypeId == shovelerComponentTypeIdTilemapTiles) {
		return shovelerWorkerSchemaComponentIdTilemapTiles;
	}
	if(componentTypeId == shovelerComponentTypeIdTilemap) {
		return shovelerWorkerSchemaComponentIdTilemap;
	}
	if(componentTypeId == shovelerComponentTypeIdSprite) {
		return shovelerWorkerSchemaComponentIdSprite;
	}
	if(componentTypeId == shovelerComponentTypeIdTileSprite) {
		return shovelerWorkerSchemaComponentIdTileSprite;
	}
	if(componentTypeId == shovelerComponentTypeIdTilemapSprite) {
		return shovelerWorkerSchemaComponentIdTilemapSprite;
	}
	if(componentTypeId == shovelerComponentTypeIdTextureSprite) {
		return shovelerWorkerSchemaComponentIdTextureSprite;
	}
	if(componentTypeId == shovelerComponentTypeIdTileSpriteAnimation) {
		return shovelerWorkerSchemaComponentIdTileSpriteAnimation;
	}
	if(componentTypeId == shovelerComponentTypeIdCanvas) {
		return shovelerWorkerSchemaComponentIdCanvas;
	}
	if(componentTypeId == shovelerComponentTypeIdDrawable) {
		return shovelerWorkerSchemaComponentIdDrawable;
	}
	if(componentTypeId == shovelerComponentTypeIdMaterial) {
		return shovelerWorkerSchemaComponentIdMaterial;
	}
	if(componentTypeId == shovelerComponentTypeIdModel) {
		return shovelerWorkerSchemaComponentIdModel;
	}
	if(componentTypeId == shovelerComponentTypeIdLight) {
		return shovelerWorkerSchemaComponentIdLight;
	}
	if(componentTypeId == shovelerComponentTypeIdSampler) {
		return shovelerWorkerSchemaComponentIdSampler;
	}
	if(componentTypeId == shovelerComponentTypeIdTilemapColliders) {
		return shovelerWorkerSchemaComponentIdTilemapColliders;
	}
	if(componentTypeId == shovelerComponentTypeIdFont) {
		return shovelerWorkerSchemaComponentIdFont;
	}
	if(componentTypeId == shovelerComponentTypeIdFontAtlas) {
		return shovelerWorkerSchemaComponentIdFontAtlas;
	}
	if(componentTypeId == shovelerComponentTypeIdFontAtlasTexture) {
		return shovelerWorkerSchemaComponentIdFontAtlasTexture;
	}
	if(componentTypeId == shovelerComponentTypeIdTextTextureRenderer) {
		return shovelerWorkerSchemaComponentIdTextTextureRenderer;
	}
	return 0;
}

void shovelerClientRegisterViewComponentTypes(ShovelerView *view)
{
	shovelerViewAddComponentType(view, shovelerComponentCreateCanvasType());
	shovelerViewAddComponentType(view, shovelerComponentCreateClientType());
	shovelerViewAddComponentType(view, shovelerComponentCreateDrawableType());
	shovelerViewAddComponentType(view, shovelerComponentCreateImageType());
	shovelerViewAddComponentType(view, shovelerComponentCreateLightType());
	shovelerViewAddComponentType(view, shovelerComponentCreateMaterialType());
	shovelerViewAddComponentType(view, shovelerComponentCreateModelType());
	shovelerViewAddComponentType(view, shovelerComponentCreatePositionType());
	shovelerViewAddComponentType(view, shovelerComponentCreateResourceType());
	shovelerViewAddComponentType(view, shovelerComponentCreateSamplerType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTextureType());
	shovelerViewAddComponentType(view, shovelerComponentCreateSpriteType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTileSpriteType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTileSpriteAnimationType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapCollidersType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapSpriteType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapTilesType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilesetType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTextureSpriteType());
	shovelerViewAddComponentType(view, shovelerComponentCreateFontType());
	shovelerViewAddComponentType(view, shovelerComponentCreateFontAtlasType());
	shovelerViewAddComponentType(view, shovelerComponentCreateFontAtlasTextureType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTextTextureRendererType());
}

void shovelerClientApplyComponentData(ShovelerView *view, ShovelerComponent *component, Schema_ComponentData *componentData, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_Object *fields = Schema_GetComponentDataFields(componentData);

	for(int optionId = 0; optionId < component->type->numConfigurationOptions; optionId++) {
		Schema_FieldId fieldId = optionId + 1;
		ShovelerComponentTypeConfigurationOption *configurationOption = &component->type->configurationOptions[optionId];

		updateComponentConfigurationOption(component, configurationOption, optionId, fields, fieldId, configurationOption->isOptional);
	}
}

void shovelerClientApplyComponentUpdate(ShovelerView *view, ShovelerComponent *component, Schema_ComponentUpdate *componentUpdate, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_Object *fields = Schema_GetComponentUpdateFields(componentUpdate);

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
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	}

	for(int optionId = 0; optionId < component->type->numConfigurationOptions; optionId++) {
		Schema_FieldId fieldId = optionId + 1;
		ShovelerComponentTypeConfigurationOption *configurationOption = &component->type->configurationOptions[optionId];

		updateComponentConfigurationOption(component, configurationOption, optionId, fields, fieldId, /* clear_if_not_set */ false);
	}
}

Schema_ComponentUpdate *shovelerClientCreateComponentUpdate(ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value)
{
	Schema_ComponentUpdate *update = Schema_CreateComponentUpdate();
	Schema_Object *fields = Schema_GetComponentUpdateFields(update);

	Schema_FieldId fieldId = 0;
	for(int optionId = 0; optionId < component->type->numConfigurationOptions; optionId++) {
		if(&component->type->configurationOptions[optionId] == configurationOption) {
			fieldId = optionId + 1;
			break;
		}
	}

	if(fieldId == 0) {
		shovelerLogWarning("Tried to update entity %lld component %s configuration option %s which couldn't be found.", component->entityId, component->type->id, configurationOption->name);
		return update;
	}

	switch(configurationOption->type) {
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_ENTITY_ID: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_AddEntityId(fields, fieldId, value->entityIdValue);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_ENTITY_ID_ARRAY: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_AddEntityIdList(fields, fieldId, (const Schema_EntityId *) value->entityIdArrayValue.entityIds, value->entityIdArrayValue.size);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_FLOAT: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_AddFloat(fields, fieldId, value->floatValue);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_BOOL: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_AddBool(fields, fieldId, value->boolValue);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_INT: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_AddInt32(fields, fieldId, value->intValue);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_STRING: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				size_t valueLength = strlen(value->stringValue);
				uint32_t bufferSize = (uint32_t) valueLength * sizeof(char);
				if(valueLength > UINT_MAX || bufferSize > (uint32_t) valueLength) {
					shovelerLogWarning("Serializing entity %lld component %s field %s update of length %zu would cause overflow, ignoring.", component->entityId, component->type->id, valueLength);
					break;
				}

				uint8_t *buffer = Schema_AllocateBuffer(fields, bufferSize);
				memcpy(buffer, value->stringValue, bufferSize);
				Schema_AddBytes(fields, fieldId, buffer, bufferSize);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR2: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_Object *vector2 = Schema_AddObject(fields, fieldId);
				Schema_AddFloat(vector2, shovelerWorkerSchemaVector2FieldIdX, value->vector2Value.values[0]);
				Schema_AddFloat(vector2, shovelerWorkerSchemaVector2FieldIdY, value->vector2Value.values[1]);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR3: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_Object *vector3 = Schema_AddObject(fields, fieldId);
				Schema_AddFloat(vector3, shovelerWorkerSchemaVector3FieldIdX, value->vector3Value.values[0]);
				Schema_AddFloat(vector3, shovelerWorkerSchemaVector3FieldIdY, value->vector3Value.values[1]);
				Schema_AddFloat(vector3, shovelerWorkerSchemaVector3FieldIdZ, value->vector3Value.values[2]);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR4: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_Object *vector4 = Schema_AddObject(fields, fieldId);
				Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdX, value->vector4Value.values[0]);
				Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdY, value->vector4Value.values[1]);
				Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdZ, value->vector4Value.values[2]);
				Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdW, value->vector4Value.values[3]);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_BYTES: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				size_t valueLength = strlen(value->stringValue);
				uint32_t bufferSize = (uint32_t) valueLength * sizeof(char);
				if(valueLength > UINT_MAX || bufferSize > (uint32_t) valueLength) {
					shovelerLogWarning("Serializing entity %lld component %s field %s update of length %zu would cause overflow, ignoring.", component->entityId, component->type->id, valueLength);
					break;
				}

				uint8_t *buffer = Schema_AllocateBuffer(fields, bufferSize);
				memcpy(buffer, value->stringValue, bufferSize);
				Schema_AddBytes(fields, fieldId, buffer, bufferSize);
			}
		} break;
	}

	return update;
}

Schema_ComponentUpdate *shovelerClientCreateImprobablePositionUpdate(ShovelerVector3 position, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_ComponentUpdate *update = Schema_CreateComponentUpdate();
	Schema_Object *fields = Schema_GetComponentUpdateFields(update);

	ShovelerVector3 coordinatesValue = shovelerVector3(
		shovelerCoordinateMap(position, mappingX),
		shovelerCoordinateMap(position, mappingY),
		shovelerCoordinateMap(position, mappingZ));

	Schema_Object *coordinates = Schema_AddObject(fields, shovelerWorkerSchemaImprobablePositionFieldIdCoords);
	Schema_AddDouble(coordinates, shovelerWorkerSchemaImprobableCoordinatesFieldIdX, coordinatesValue.values[0]);
	Schema_AddDouble(coordinates, shovelerWorkerSchemaImprobableCoordinatesFieldIdY, coordinatesValue.values[1]);
	Schema_AddDouble(coordinates, shovelerWorkerSchemaImprobableCoordinatesFieldIdZ, coordinatesValue.values[2]);

	return update;
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
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		long long int entityIdValue = Schema_GetEntityId(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionEntityId(component, optionId, entityIdValue);

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to entity ID value %lld.", component->entityId, component->type->id, configurationOption->name, entityIdValue);
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

	shovelerLogTrace("Updated entity %lld component '%s' option '%s' to entity ID list value with %d element(s).", component->entityId, component->type->id, configurationOption->name, numEntityIds);
}

static void updateFloatConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetFloatCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		float floatValue = Schema_GetFloat(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionFloat(component, optionId, floatValue);

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to float value %f.", component->entityId, component->type->id, configurationOption->name, floatValue);
	}
}

static void updateBoolConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetBoolCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		bool boolValue = Schema_GetBool(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionBool(component, optionId, boolValue);

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to bool value %s.", component->entityId, component->type->id, configurationOption->name, boolValue ? "true" : "false");
	}
}

static void updateIntConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetInt32Count(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		int intValue = Schema_GetInt32(fields, fieldId);
		shovelerComponentUpdateCanonicalConfigurationOptionInt(component, optionId, intValue);

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to int value %d.", component->entityId, component->type->id, configurationOption->name, intValue);
	}
}

static void updateStringConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetBytesCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		int bytesLength = (int) Schema_GetBytesLength(fields, fieldId);
		const char *bytesValue = (const char *) Schema_GetBytes(fields, fieldId);

		GString *stringValue = g_string_new("");
		g_string_append_len(stringValue, bytesValue, bytesLength);

		shovelerComponentUpdateCanonicalConfigurationOptionString(component, optionId, stringValue->str);

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to string value '%s'.", component->entityId, component->type->id, configurationOption->name, stringValue->str);

		g_string_free(stringValue, /* free_segment */ true);
	}
}

static void updateVector2ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetObjectCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		Schema_Object *vector2 = Schema_GetObject(fields, fieldId);
		float x = Schema_GetFloat(vector2, shovelerWorkerSchemaVector2FieldIdX);
		float y = Schema_GetFloat(vector2, shovelerWorkerSchemaVector2FieldIdY);

		shovelerComponentUpdateCanonicalConfigurationOptionVector2(component, optionId, shovelerVector2(x, y));

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to vector2 value (%f, %f).", component->entityId, component->type->id, configurationOption->name, x, y);
	}
}

static void updateVector3ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetObjectCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		Schema_Object *vector3 = Schema_GetObject(fields, fieldId);
		float x = Schema_GetFloat(vector3, shovelerWorkerSchemaVector3FieldIdX);
		float y = Schema_GetFloat(vector3, shovelerWorkerSchemaVector3FieldIdY);
		float z = Schema_GetFloat(vector3, shovelerWorkerSchemaVector3FieldIdZ);

		shovelerComponentUpdateCanonicalConfigurationOptionVector3(component, optionId, shovelerVector3(x, y, z));

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to vector3 value (%f, %f, %f).", component->entityId, component->type->id, configurationOption->name, x, y, z);
	}
}

static void updateVector4ConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetObjectCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		Schema_Object *vector4 = Schema_GetObject(fields, fieldId);
		float x = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdX);
		float y = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdY);
		float z = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdZ);
		float w = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdW);

		shovelerComponentUpdateCanonicalConfigurationOptionVector4(component, optionId, shovelerVector4(x, y, z, w));

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to vector4 value (%f, %f, %f, %f).", component->entityId, component->type->id, configurationOption->name, x, y, z, w);
	}
}

static void updateBytesConfigurationOption(ShovelerComponent *component, ShovelerComponentTypeConfigurationOption *configurationOption, int optionId, Schema_Object *fields, int fieldId, bool clear_if_not_set)
{
	if(Schema_GetBytesCount(fields, fieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

		shovelerComponentClearConfigurationOption(component, optionId, /* isCanonical */ true);
		shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, configurationOption->name);
	} else {
		int bytesLength = (int) Schema_GetBytesLength(fields, fieldId);
		const unsigned char *bytesValue = Schema_GetBytes(fields, fieldId);

		shovelerComponentUpdateCanonicalConfigurationOptionBytes(component, optionId, bytesValue, bytesLength);

		shovelerLogTrace("Updated entity %lld component '%s' option '%s' to %d bytes value.", component->entityId, component->type->id, configurationOption->name, bytesLength);
	}
}
