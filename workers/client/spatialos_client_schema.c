#include "spatialos_client_schema.h"

#include <limits.h> // INT_MAX
#include <stddef.h>
#include <stdlib.h>
#include <string.h> // strlen memcpy

#include <shoveler/component.h>
#include <shoveler/component_field.h>
#include <shoveler/component_type.h>
#include <shoveler/log.h>
#include <shoveler/schema/base.h>
#include <shoveler/schema/opengl.h>
#include <shoveler/spatialos_schema.h>
#include <shoveler/world.h>

static void updateComponentField(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateEntityIdfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateEntityIdArrayfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId);
static void updateFloatfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateBoolfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateIntfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateStringfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateVector2field(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateVector3field(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateVector4field(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);
static void updateBytesfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set);

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
    if(componentTypeId == shovelerComponentTypeIdTextSprite) {
        return shovelerWorkerSchemaComponentIdTextSprite;
    }
	return 0;
}

void shovelerClientApplyComponentData(ShovelerWorld *world, ShovelerComponent *component, Schema_ComponentData *componentData, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_Object *fields = Schema_GetComponentDataFields(componentData);

    for(int fieldId = 0; fieldId < component->type->numFields; fieldId++) {
        Schema_FieldId spatialosFieldId = fieldId + 1;
        ShovelerComponentField *field = &component->type->fields[fieldId];

        updateComponentField(component, field, fieldId, fields, spatialosFieldId, field->isOptional);
	}
}

void shovelerClientApplyComponentUpdate(ShovelerWorld *world, ShovelerComponent *component, Schema_ComponentUpdate *componentUpdate, ShovelerCoordinateMapping mappingX, ShovelerCoordinateMapping mappingY, ShovelerCoordinateMapping mappingZ)
{
	Schema_Object *fields = Schema_GetComponentUpdateFields(componentUpdate);

	uint32_t cleared_field_count = Schema_GetComponentUpdateClearedFieldCount(componentUpdate);
	assert(cleared_field_count <= INT_MAX);
	for(int i = 0; i < (int) cleared_field_count; i++) {
        Schema_FieldId spatialosFieldId = Schema_IndexComponentUpdateClearedField(componentUpdate, i);
        assert(spatialosFieldId < INT_MAX);
        int fieldId = (int) spatialosFieldId - 1;
        ShovelerComponentField *field = &component->type->fields[fieldId];

        if(field->type != SHOVELER_COMPONENT_FIELD_TYPE_ENTITY_ID_ARRAY && !field->isOptional) {
            shovelerLogWarning("Received clear for non-optional entity %lld component '%s' option '%s', ignoring.", component->entityId, component->type->id, field->name);
			continue;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	}

    for(int fieldId = 0; fieldId < component->type->numFields; fieldId++) {
        Schema_FieldId spatialosFieldId = fieldId + 1;
        ShovelerComponentField *field = &component->type->fields[fieldId];

        updateComponentField(component, field, fieldId, fields, spatialosFieldId, /* clear_if_not_set */ false);
	}
}

Schema_ComponentUpdate *shovelerClientCreateComponentUpdate(ShovelerComponent *component, const ShovelerComponentField *field, const ShovelerComponentFieldValue *value)
{
	Schema_ComponentUpdate *update = Schema_CreateComponentUpdate();
	Schema_Object *fields = Schema_GetComponentUpdateFields(update);

    Schema_FieldId spatialosFieldId = 0;
    for(int fieldId = 0; fieldId < component->type->numFields; fieldId++) {
        if(&component->type->fields[fieldId] == field) {
            spatialosFieldId = fieldId + 1;
			break;
		}
	}

    if(spatialosFieldId == 0) {
        shovelerLogWarning("Tried to update entity %lld component %s configuration option %s which couldn't be found.", component->entityId, component->type->id, field->name);
		return update;
	}

    switch(field->type) {
        case SHOVELER_COMPONENT_FIELD_TYPE_ENTITY_ID: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_AddEntityId(fields, spatialosFieldId, value->entityIdValue);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_ENTITY_ID_ARRAY: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_AddEntityIdList(fields, spatialosFieldId, (const Schema_EntityId *) value->entityIdArrayValue.entityIds, value->entityIdArrayValue.size);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_FLOAT: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_AddFloat(fields, spatialosFieldId, value->floatValue);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_BOOL: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_AddBool(fields, spatialosFieldId, value->boolValue);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_INT: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_AddInt32(fields, spatialosFieldId, value->intValue);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_STRING: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
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
                Schema_AddBytes(fields, spatialosFieldId, buffer, bufferSize);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_VECTOR2: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_Object *vector2 = Schema_AddObject(fields, spatialosFieldId);
                Schema_AddFloat(vector2, shovelerWorkerSchemaVector2FieldIdX, value->vector2Value.values[0]);
                Schema_AddFloat(vector2, shovelerWorkerSchemaVector2FieldIdY, value->vector2Value.values[1]);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_VECTOR3: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_Object *vector3 = Schema_AddObject(fields, spatialosFieldId);
                Schema_AddFloat(vector3, shovelerWorkerSchemaVector3FieldIdX, value->vector3Value.values[0]);
                Schema_AddFloat(vector3, shovelerWorkerSchemaVector3FieldIdY, value->vector3Value.values[1]);
                Schema_AddFloat(vector3, shovelerWorkerSchemaVector3FieldIdZ, value->vector3Value.values[2]);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_VECTOR4: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
			} else {
				assert(value != NULL);
                Schema_Object *vector4 = Schema_AddObject(fields, spatialosFieldId);
                Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdX, value->vector4Value.values[0]);
                Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdY, value->vector4Value.values[1]);
                Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdZ, value->vector4Value.values[2]);
                Schema_AddFloat(vector4, shovelerWorkerSchemaVector4FieldIdW, value->vector4Value.values[3]);
			}
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_BYTES: {
            if(field->isOptional && value == NULL) {
                Schema_AddComponentUpdateClearedField(update, spatialosFieldId);
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
                Schema_AddBytes(fields, spatialosFieldId, buffer, bufferSize);
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

static void updateComponentField(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    switch(field->type) {
        case SHOVELER_COMPONENT_FIELD_TYPE_ENTITY_ID: {
            updateEntityIdfield(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_ENTITY_ID_ARRAY: {
            updateEntityIdArrayfield(component, field, fieldId, fields, spatialosFieldId);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_FLOAT: {
            updateFloatfield(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_BOOL: {
            updateBoolfield(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_INT: {
            updateIntfield(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_STRING: {
            updateStringfield(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_VECTOR2: {
            updateVector2field(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_VECTOR3: {
            updateVector3field(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_VECTOR4: {
            updateVector4field(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
        case SHOVELER_COMPONENT_FIELD_TYPE_BYTES: {
            updateBytesfield(component, field, fieldId, fields, spatialosFieldId, clear_if_not_set);
		} break;
	}
}

static void updateEntityIdfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetEntityIdCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        long long int entityIdValue = Schema_GetEntityId(fields, spatialosFieldId);
        shovelerComponentUpdateCanonicalFieldEntityId(component, fieldId, entityIdValue);

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to entity ID value %lld.", component->entityId, component->type->id, field->name, entityIdValue);
	}
}

static void updateEntityIdArrayfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId)
{
    int numEntityIds = Schema_GetEntityIdCount(fields, spatialosFieldId);

	long long int *entityIdArrayValue = malloc(numEntityIds * sizeof(long long int));
	for(int j = 0; j < numEntityIds; j++) {
        entityIdArrayValue[j] = Schema_IndexEntityId(fields, spatialosFieldId, j);
	}

    shovelerComponentUpdateCanonicalFieldEntityIdArray(component, fieldId, entityIdArrayValue, numEntityIds);

	free(entityIdArrayValue);

    shovelerLogTrace("Updated entity %lld component '%s' option '%s' to entity ID list value with %d element(s).", component->entityId, component->type->id, field->name, numEntityIds);
}

static void updateFloatfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetFloatCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        float floatValue = Schema_GetFloat(fields, spatialosFieldId);
        shovelerComponentUpdateCanonicalFieldFloat(component, fieldId, floatValue);

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to float value %f.", component->entityId, component->type->id, field->name, floatValue);
	}
}

static void updateBoolfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetBoolCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        bool boolValue = Schema_GetBool(fields, spatialosFieldId);
        shovelerComponentUpdateCanonicalFieldBool(component, fieldId, boolValue);

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to bool value %s.", component->entityId, component->type->id, field->name, boolValue ? "true" : "false");
	}
}

static void updateIntfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetInt32Count(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        int intValue = Schema_GetInt32(fields, spatialosFieldId);
        shovelerComponentUpdateCanonicalFieldInt(component, fieldId, intValue);

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to int value %d.", component->entityId, component->type->id, field->name, intValue);
	}
}

static void updateStringfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetBytesCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        int bytesLength = (int) Schema_GetBytesLength(fields, spatialosFieldId);
        const char *bytesValue = (const char *) Schema_GetBytes(fields, spatialosFieldId);

		GString *stringValue = g_string_new("");
		g_string_append_len(stringValue, bytesValue, bytesLength);

        shovelerComponentUpdateCanonicalFieldString(component, fieldId, stringValue->str);

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to string value '%s'.", component->entityId, component->type->id, field->name, stringValue->str);

		g_string_free(stringValue, /* free_segment */ true);
	}
}

static void updateVector2field(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetObjectCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        Schema_Object *vector2 = Schema_GetObject(fields, spatialosFieldId);
        float x = Schema_GetFloat(vector2, shovelerWorkerSchemaVector2FieldIdX);
        float y = Schema_GetFloat(vector2, shovelerWorkerSchemaVector2FieldIdY);

        shovelerComponentUpdateCanonicalFieldVector2(component, fieldId, shovelerVector2(x, y));

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to vector2 value (%f, %f).", component->entityId, component->type->id, field->name, x, y);
	}
}

static void updateVector3field(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetObjectCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        Schema_Object *vector3 = Schema_GetObject(fields, spatialosFieldId);
        float x = Schema_GetFloat(vector3, shovelerWorkerSchemaVector3FieldIdX);
        float y = Schema_GetFloat(vector3, shovelerWorkerSchemaVector3FieldIdY);
        float z = Schema_GetFloat(vector3, shovelerWorkerSchemaVector3FieldIdZ);

        shovelerComponentUpdateCanonicalFieldVector3(component, fieldId, shovelerVector3(x, y, z));

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to vector3 value (%f, %f, %f).", component->entityId, component->type->id, field->name, x, y, z);
	}
}

static void updateVector4field(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetObjectCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        Schema_Object *vector4 = Schema_GetObject(fields, spatialosFieldId);
        float x = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdX);
        float y = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdY);
        float z = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdZ);
        float w = Schema_GetFloat(vector4, shovelerWorkerSchemaVector4FieldIdW);

        shovelerComponentUpdateCanonicalFieldVector4(component, fieldId, shovelerVector4(x, y, z, w));

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to vector4 value (%f, %f, %f, %f).", component->entityId, component->type->id, field->name, x, y, z, w);
	}
}

static void updateBytesfield(ShovelerComponent *component, ShovelerComponentField *field, int fieldId, Schema_Object *fields, Schema_FieldId spatialosFieldId, bool clear_if_not_set)
{
    if(Schema_GetBytesCount(fields, spatialosFieldId) == 0) {
		if(!clear_if_not_set) {
			return;
		}

        shovelerComponentClearField(component, fieldId, /* isCanonical */ true);
        shovelerLogTrace("Cleared entity %lld component '%s' option '%s'.", component->entityId, component->type->id, field->name);
	} else {
        int bytesLength = (int) Schema_GetBytesLength(fields, spatialosFieldId);
        const unsigned char *bytesValue = Schema_GetBytes(fields, spatialosFieldId);

        shovelerComponentUpdateCanonicalFieldBytes(component, fieldId, bytesValue, bytesLength);

        shovelerLogTrace("Updated entity %lld component '%s' option '%s' to %d bytes value.", component->entityId, component->type->id, field->name, bytesLength);
	}
}
