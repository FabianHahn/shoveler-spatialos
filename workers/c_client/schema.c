#include <limits.h> // INT_MAX
#include <stddef.h>
#include <stdlib.h>
#include <string.h> // strlen memcpy

#include <shoveler/component/canvas.h>
#include <shoveler/component/chunk.h>
#include <shoveler/component/chunk_layer.h>
#include <shoveler/component/client.h>
#include <shoveler/component/drawable.h>
#include <shoveler/component/image.h>
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
		case 13377:
			return shovelerComponentTypeIdImage;
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

const char *shovelerClientResolveSpecialComponentId(int componentId)
{
	switch(componentId) {
		case 50:
			return "EntityAcl";
		case 53:
			return "Metadata";
		// 54: we treat position as a regular view component even though it is part of the standard library
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
		case 1336:
			return "ClientHeartbeat";
		default:
			return NULL;
	}
}

int shovelerClientResolveComponentSchemaId(const char *componentTypeId)
{
	if(componentTypeId == shovelerComponentTypeIdPosition) {
		return 54;
	}
	if(componentTypeId == shovelerComponentTypeIdClient) {
		return 1335;
	}
	if(componentTypeId == shovelerComponentTypeIdResource) {
		return 1337;
	}
	if(componentTypeId == shovelerComponentTypeIdImage) {
		return 13377;
	}
	if(componentTypeId == shovelerComponentTypeIdTexture) {
		return 1338;
	}
	if(componentTypeId == shovelerComponentTypeIdTileset) {
		return 1339;
	}
	if(componentTypeId == shovelerComponentTypeIdTilemapTiles) {
		return 1340;
	}
	if(componentTypeId == shovelerComponentTypeIdTilemap) {
		return 1341;
	}
	if(componentTypeId == shovelerComponentTypeIdTileSprite) {
		return 1342;
	}
	if(componentTypeId == shovelerComponentTypeIdTileSpriteAnimation) {
		return 1343;
	}
	if(componentTypeId == shovelerComponentTypeIdCanvas) {
		return 1344;
	}
	if(componentTypeId == shovelerComponentTypeIdChunkLayer) {
		return 1345;
	}
	if(componentTypeId == shovelerComponentTypeIdDrawable) {
		return 1346;
	}
	if(componentTypeId == shovelerComponentTypeIdMaterial) {
		return 1347;
	}
	if(componentTypeId == shovelerComponentTypeIdModel) {
		return 1348;
	}
	if(componentTypeId == shovelerComponentTypeIdLight) {
		return 1349;
	}
	if(componentTypeId == shovelerComponentTypeIdSampler) {
		return 13381;
	}
	if(componentTypeId == shovelerComponentTypeIdTilemapColliders) {
		return 134132;
	}
	if(componentTypeId == shovelerComponentTypeIdChunk) {
		return 13451337;
	}
	return 0;
}

void shovelerClientRegisterViewComponentTypes(ShovelerView *view)
{
	shovelerViewAddComponentType(view, shovelerComponentCreateCanvasType());
	shovelerViewAddComponentType(view, shovelerComponentCreateChunkType());
	shovelerViewAddComponentType(view, shovelerComponentCreateChunkLayerType());
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
	shovelerViewAddComponentType(view, shovelerComponentCreateTileSpriteType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTileSpriteAnimationType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapType());
	shovelerViewAddComponentType(view, shovelerComponentCreateTilemapCollidersType());
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

Schema_ComponentUpdate *shovelerClientCreateComponentUpdate(ShovelerComponent *component, const ShovelerComponentTypeConfigurationOption *configurationOption, const ShovelerComponentConfigurationValue *value, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_ComponentUpdate *update = Schema_CreateComponentUpdate();
	Schema_Object *fields = Schema_GetComponentUpdateFields(update);

	// special case position updates
	if(component->type->id == shovelerComponentTypeIdPosition) {
		assert(configurationOption->type == SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR3);

		// TODO: inverse mapping here?
		ShovelerVector3 coordinatesValue = shovelerVector3(
			shovelerCoordinateMap(value->vector3Value, mappingX),
			shovelerCoordinateMap(value->vector3Value, mappingY),
			shovelerCoordinateMap(value->vector3Value, mappingZ));

		Schema_Object *coordinates = Schema_AddObject(fields, /* fieldId */ 1);
		Schema_AddDouble(coordinates, /* fieldId */ 1, coordinatesValue.values[0]);
		Schema_AddDouble(coordinates, /* fieldId */ 2, coordinatesValue.values[1]);
		Schema_AddDouble(coordinates, /* fieldId */ 3, coordinatesValue.values[2]);

		return update;
	}

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
				Schema_AddFloat(vector2, /* fieldId */ 1, value->vector2Value.values[0]);
				Schema_AddFloat(vector2, /* fieldId */ 2, value->vector2Value.values[1]);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR3: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_Object *vector3 = Schema_AddObject(fields, fieldId);
				Schema_AddFloat(vector3, /* fieldId */ 1, value->vector3Value.values[0]);
				Schema_AddFloat(vector3, /* fieldId */ 2, value->vector3Value.values[1]);
				Schema_AddFloat(vector3, /* fieldId */ 3, value->vector3Value.values[2]);
			}
		} break;
		case SHOVELER_COMPONENT_CONFIGURATION_OPTION_TYPE_VECTOR4: {
			if(configurationOption->isOptional && value == NULL) {
				Schema_AddComponentUpdateClearedField(update, fieldId);
			} else {
				assert(value != NULL);
				Schema_Object *vector4 = Schema_AddObject(fields, fieldId);
				Schema_AddFloat(vector4, /* fieldId */ 1, value->vector4Value.values[0]);
				Schema_AddFloat(vector4, /* fieldId */ 2, value->vector4Value.values[1]);
				Schema_AddFloat(vector4, /* fieldId */ 3, value->vector4Value.values[2]);
				Schema_AddFloat(vector4, /* fieldId */ 4, value->vector4Value.values[3]);
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
