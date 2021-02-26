#include "shoveler/schema.h"

#include <assert.h> // assert
#include <stdlib.h> // NULL
#include <string.h> // strlen

#include <shoveler/component/position.h>

// FIXME include from component data definition instead
typedef enum {
	SHOVELER_COMPONENT_DRAWABLE_TYPE_CUBE,
	SHOVELER_COMPONENT_DRAWABLE_TYPE_QUAD,
	SHOVELER_COMPONENT_DRAWABLE_TYPE_POINT,
	SHOVELER_COMPONENT_DRAWABLE_TYPE_TILES,
} ShovelerComponentDrawableType;

const char *shovelerWorkerSchemaResolveSpecialComponentId(int componentId)
{
	switch(componentId) {
		case shovelerWorkerSchemaComponentIdImprobableMetadata:
			return "Metadata";
		case shovelerWorkerSchemaComponentIdImprobablePosition:
			return "Position";
		case shovelerWorkerSchemaComponentIdImprobablePersistence:
			return "Persistence";
		case shovelerWorkerSchemaComponentIdImprobableInterest:
			return "Interest";
		case shovelerWorkerSchemaComponentIdImprobableSystem:
			return "System";
		case shovelerWorkerSchemaComponentIdImprobableWorker:
			return "Worker";
		case shovelerWorkerSchemaComponentIdImprobablePlayerClient:
			return "PlayerClient";
		case shovelerWorkerSchemaComponentIdImprobableAuthorityDelegation:
			return "AuthorityDelegation";
			// special shoveler components below
		case shovelerWorkerSchemaComponentIdBootstrap:
			return "Bootstrap";
		case shovelerWorkerSchemaComponentIdClientInfo:
			return "ClientInfo";
		case shovelerWorkerSchemaComponentIdClientHeartbeatPing:
			return "ClientHeartbeatPing";
		case shovelerWorkerSchemaComponentIdClientHeartbeatPong:
			return "ClientHeartbeatPong";
		default:
			return NULL;
	}
}

Worker_ComponentData shovelerWorkerSchemaCreateImprobableMetadataComponent(const char *staticEntityType)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdImprobableMetadata;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *metadata = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddBytes(metadata, shovelerWorkerSchemaImprobableMetadataFieldIdEntityType, (const uint8_t *) staticEntityType, strlen(staticEntityType));
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateImprobablePersistenceComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdImprobablePersistence;
	componentData.schema_type = Schema_CreateComponentData();
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateImprobablePositionComponent(double x, double y, double z)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdImprobablePosition;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *improbablePosition = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_Object *coords = Schema_AddObject(improbablePosition, shovelerWorkerSchemaImprobablePositionFieldIdCoords);
	Schema_AddDouble(coords, shovelerWorkerSchemaImprobableCoordinatesFieldIdX, x);
	Schema_AddDouble(coords, shovelerWorkerSchemaImprobableCoordinatesFieldIdY, y);
	Schema_AddDouble(coords, shovelerWorkerSchemaImprobableCoordinatesFieldIdZ, z);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdImprobableAuthorityDelegation;
	componentData.schema_type = Schema_CreateComponentData();
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateImprobableInterestComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdImprobableInterest;
	componentData.schema_type = Schema_CreateComponentData();
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreatePositionComponent(ShovelerVector3 positionCoordinates)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdPosition;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *position = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(position, shovelerWorkerSchemaPositionFieldIdPositionType, SHOVELER_COMPONENT_POSITION_TYPE_ABSOLUTE);
	Schema_Object *coordinates = Schema_AddObject(position, shovelerWorkerSchemaPositionFieldIdCoordinates);
	Schema_AddFloat(coordinates, shovelerWorkerSchemaVector3FieldIdX, positionCoordinates.values[0]);
	Schema_AddFloat(coordinates, shovelerWorkerSchemaVector3FieldIdY, positionCoordinates.values[1]);
	Schema_AddFloat(coordinates, shovelerWorkerSchemaVector3FieldIdZ, positionCoordinates.values[2]);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateBootstrapComponent() {
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdBootstrap;
	componentData.schema_type = Schema_CreateComponentData();
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateDrawableCubeComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdDrawable;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *drawable = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(drawable, shovelerWorkerSchemaDrawableFieldIdType, shovelerWorkerSchemaDrawableTypeCube);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateDrawableQuadComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdDrawable;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *drawable = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(drawable, shovelerWorkerSchemaDrawableFieldIdType, shovelerWorkerSchemaDrawableTypeQuad);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateDrawablePointComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdDrawable;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *drawable = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(drawable, shovelerWorkerSchemaDrawableFieldIdType, shovelerWorkerSchemaDrawableTypePoint);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateDrawableTilesComponent(int tilesWidth, int tilesHeight)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdDrawable;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *drawable = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(drawable, shovelerWorkerSchemaDrawableFieldIdType, shovelerWorkerSchemaDrawableTypePoint);
	Schema_AddInt32(drawable, shovelerWorkerSchemaDrawableFieldIdTilesWidth, tilesWidth);
	Schema_AddInt32(drawable, shovelerWorkerSchemaDrawableFieldIdTilesHeight, tilesHeight);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateMaterialColorComponent(ShovelerVector4 colorRgba)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdMaterial;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *material = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(material, shovelerWorkerSchemaMaterialFieldIdType, shovelerWorkerSchemaMaterialTypeColor);
	Schema_Object *color = Schema_AddObject(material, shovelerWorkerSchemaMaterialFieldIdColor);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdX, colorRgba.values[0]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdY, colorRgba.values[1]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdZ, colorRgba.values[2]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdW, colorRgba.values[3]);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateMaterialParticleComponent(ShovelerVector4 colorRgba)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdMaterial;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *material = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(material, shovelerWorkerSchemaMaterialFieldIdType, shovelerWorkerSchemaMaterialTypeParticle);
	Schema_Object *color = Schema_AddObject(material, shovelerWorkerSchemaMaterialFieldIdColor);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdX, colorRgba.values[0]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdY, colorRgba.values[1]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdZ, colorRgba.values[2]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector4FieldIdW, colorRgba.values[3]);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateMaterialTileSpriteComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdMaterial;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *material = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(material, shovelerWorkerSchemaMaterialFieldIdType, shovelerWorkerSchemaMaterialTypeTileSprite);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateMaterialTilemapComponent()
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdMaterial;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *material = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(material, shovelerWorkerSchemaMaterialFieldIdType, shovelerWorkerSchemaMaterialTypeTilemap);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateMaterialCanvasComponent(
	Worker_EntityId canvas, ShovelerVector2 regionPosition, ShovelerVector2 regionSize)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdMaterial;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *material = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(material, shovelerWorkerSchemaMaterialFieldIdType, shovelerWorkerSchemaMaterialTypeCanvas);
	Schema_AddEntityId(material, shovelerWorkerSchemaMaterialFieldIdCanvas, canvas);
	Schema_Object *regionPositionObject = Schema_AddObject(material, shovelerWorkerSchemaMaterialFieldIdCanvasRegionPosition);
	Schema_AddFloat(regionPositionObject, shovelerWorkerSchemaVector2FieldIdX, regionPosition.values[0]);
	Schema_AddFloat(regionPositionObject, shovelerWorkerSchemaVector2FieldIdY, regionPosition.values[1]);
	Schema_Object *regionSizeObject = Schema_AddObject(material, shovelerWorkerSchemaMaterialFieldIdCanvasRegionSize);
	Schema_AddFloat(regionSizeObject, shovelerWorkerSchemaVector2FieldIdX, regionSize.values[0]);
	Schema_AddFloat(regionSizeObject, shovelerWorkerSchemaVector2FieldIdY, regionSize.values[1]);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateModelComponent(
	Worker_EntityId position,
	Worker_EntityId drawable,
	Worker_EntityId material,
	ShovelerVector3 rotation,
	ShovelerVector3 scale,
	bool visible,
	bool emitter,
	bool castsShadow,
	int polygonMode /* TODO: typesafe enum */)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdModel;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *model = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(model, shovelerWorkerSchemaModelFieldIdPosition, position);
	Schema_AddEntityId(model, shovelerWorkerSchemaModelFieldIdDrawable, drawable);
	Schema_AddEntityId(model, shovelerWorkerSchemaModelFieldIdMaterial, material);
	Schema_Object *rotationObject = Schema_AddObject(model, shovelerWorkerSchemaModelFieldIdRotation);
	Schema_AddFloat(rotationObject, shovelerWorkerSchemaVector3FieldIdX, rotation.values[0]);
	Schema_AddFloat(rotationObject, shovelerWorkerSchemaVector3FieldIdY, rotation.values[1]);
	Schema_AddFloat(rotationObject, shovelerWorkerSchemaVector3FieldIdZ, rotation.values[2]);
	Schema_Object *scaleObject = Schema_AddObject(model, shovelerWorkerSchemaModelFieldIdScale);
	Schema_AddFloat(scaleObject, shovelerWorkerSchemaVector3FieldIdX, scale.values[0]);
	Schema_AddFloat(scaleObject, shovelerWorkerSchemaVector3FieldIdY, scale.values[1]);
	Schema_AddFloat(scaleObject, shovelerWorkerSchemaVector3FieldIdZ, scale.values[2]);
	Schema_AddBool(model, shovelerWorkerSchemaModelFieldIdVisible, visible);
	Schema_AddBool(model, shovelerWorkerSchemaModelFieldIdEmitter, emitter);
	Schema_AddBool(model, shovelerWorkerSchemaModelFieldIdCastsShadow, castsShadow);
	Schema_AddEnum(model, shovelerWorkerSchemaModelFieldIdPolygonMode, polygonMode);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateLightComponent(
	Worker_EntityId position,
	int lightType /* TODO: typesafe enum */,
	int width,
	int height,
	int samples,
	float ambientFactor,
	float exponentialFactor,
	ShovelerVector3 colorRgb)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdLight;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *light = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(light, shovelerWorkerSchemaLightFieldIdPosition, position);
	Schema_AddEnum(light, shovelerWorkerSchemaLightFieldIdType, lightType);
	Schema_AddInt32(light, shovelerWorkerSchemaLightFieldIdWidth, width);
	Schema_AddInt32(light, shovelerWorkerSchemaLightFieldIdHeight, height);
	Schema_AddInt32(light, shovelerWorkerSchemaLightFieldIdSamples, samples);
	Schema_AddFloat(light, shovelerWorkerSchemaLightFieldIdAmbientFactor, ambientFactor);
	Schema_AddFloat(light, shovelerWorkerSchemaLightFieldIdExponentialFactor, exponentialFactor);
	Schema_Object *color = Schema_AddObject(light, shovelerWorkerSchemaLightFieldIdColor);
	Schema_AddFloat(color, shovelerWorkerSchemaVector3FieldIdX, colorRgb.values[0]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector3FieldIdY, colorRgb.values[1]);
	Schema_AddFloat(color, shovelerWorkerSchemaVector3FieldIdZ, colorRgb.values[2]);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateResourceComponent(unsigned char *buffer, int bufferSize)
{
	assert(bufferSize > 0);

	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdResource;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *resource = Schema_GetComponentDataFields(componentData.schema_type);
	uint8_t *allocatedBuffer = Schema_AllocateBuffer(resource, bufferSize);
	memcpy(allocatedBuffer, buffer, bufferSize);
	Schema_AddBytes(resource, shovelerWorkerSchemaResourceFieldIdBuffer, allocatedBuffer, bufferSize);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateImageComponent(int format, Worker_EntityId resource)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdImage;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *image = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(image, shovelerWorkerSchemaImageFieldIdFormat, format);
	Schema_AddEntityId(image, shovelerWorkerSchemaImageFieldIdResource, resource);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateSamplerComponent(bool interpolate, bool useMipmaps, bool clamp)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdSampler;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *image = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddBool(image, shovelerWorkerSchemaSamplerFieldIdInterpolate, interpolate);
	Schema_AddBool(image, shovelerWorkerSchemaSamplerFieldIdUseMipmaps, useMipmaps);
	Schema_AddBool(image, shovelerWorkerSchemaSamplerFieldIdClamp, clamp);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTextureImageComponent(Worker_EntityId image)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTexture;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *texture = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEnum(texture, shovelerWorkerSchemaTextureFieldIdType, shovelerWorkerSchemaTextureTypeImage);
	Schema_AddEntityId(texture, shovelerWorkerSchemaTextureFieldIdImage, image);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTilesetComponent(
	Worker_EntityId image, int numColumns, int numRows, int padding)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTileset;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tileset = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(tileset, shovelerWorkerSchemaTilesetFieldIdImage, image);
	Schema_AddInt32(tileset, shovelerWorkerSchemaTilesetFieldIdNumColumns, numColumns);
	Schema_AddInt32(tileset, shovelerWorkerSchemaTilesetFieldIdNumRows, numRows);
	Schema_AddInt32(tileset, shovelerWorkerSchemaTilesetFieldIdPadding, padding);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateCanvasComponent(int numLayers)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdCanvas;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *canvas = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddInt32(canvas, shovelerWorkerSchemaCanvasFieldIdNumLayers, numLayers);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTilemapCollidersComponent(
	int numColumns, int numRows, unsigned char *colliders, int collidersSize)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTilemapColliders;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tilemapColliders = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddInt32(tilemapColliders, shovelerWorkerSchemaTilemapCollidersFieldIdNumColumns, numColumns);
	Schema_AddInt32(tilemapColliders, shovelerWorkerSchemaTilemapCollidersFieldIdNumRows, numRows);
	uint8_t *allocatedBuffer = Schema_AllocateBuffer(tilemapColliders, collidersSize);
	memcpy(allocatedBuffer, colliders, collidersSize);
	Schema_AddBytes(tilemapColliders, shovelerWorkerSchemaTilemapCollidersFieldIdColliders, allocatedBuffer, collidersSize);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTilemapTilesImageComponent(Worker_EntityId image)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTilemapTiles;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tilemapTiles = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(tilemapTiles, shovelerWorkerSchemaTilemapTilesFieldIdImage, image);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTilemapTilesDirectComponent(
	int numColumns,
	int numRows,
	unsigned char *columns,
	unsigned char *rows,
	unsigned char *ids)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTilemapTiles;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tilemapTiles = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddInt32(tilemapTiles, shovelerWorkerSchemaTilemapTilesFieldIdNumColumns, numColumns);
	Schema_AddInt32(tilemapTiles, shovelerWorkerSchemaTilemapTilesFieldIdNumRows, numRows);
	uint8_t *allocatedBufferColumns = Schema_AllocateBuffer(tilemapTiles, numColumns * numRows);
	uint8_t *allocatedBufferRows = Schema_AllocateBuffer(tilemapTiles, numColumns * numRows);
	uint8_t *allocatedBufferIds = Schema_AllocateBuffer(tilemapTiles, numColumns * numRows);
	memcpy(allocatedBufferColumns, columns, numColumns * numRows);
	memcpy(allocatedBufferRows, rows, numColumns * numRows);
	memcpy(allocatedBufferIds, ids, numColumns * numRows);
	Schema_AddBytes(tilemapTiles, shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns, allocatedBufferColumns, numColumns * numRows);
	Schema_AddBytes(tilemapTiles, shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows, allocatedBufferRows, numColumns * numRows);
	Schema_AddBytes(tilemapTiles, shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds, allocatedBufferIds, numColumns * numRows);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTilemapComponent(
	Worker_EntityId tiles,
	Worker_EntityId colliders,
	Worker_EntityId *tilesets,
	int tilesetsSize)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTilemap;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tilemapTiles = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(tilemapTiles, shovelerWorkerSchemaTilemapFieldIdTiles, tiles);
	Schema_AddEntityId(tilemapTiles, shovelerWorkerSchemaTilemapFieldIdColliders, colliders);
	for(int i = 0; i < tilesetsSize; i++) {
		Schema_AddEntityId(tilemapTiles, shovelerWorkerSchemaTilemapFieldIdTilesets, tilesets[i]);
	}
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTileSpriteComponent(
	Worker_EntityId material,
	Worker_EntityId tileset,
	int tilesetColumn,
	int tilesetRow)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTileSprite;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tileSprite = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdMaterial, material);
	Schema_AddEntityId(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdTileset, tileset);
	Schema_AddInt32(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdTilesetColumn, tilesetColumn);
	Schema_AddInt32(tileSprite, shovelerWorkerSchemaTileSpriteFieldIdTilesetRow, tilesetRow);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTilemapSpriteComponent(Worker_EntityId material, Worker_EntityId tilemap)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTilemapSprite;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tilemapSprite = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(tilemapSprite, shovelerWorkerSchemaTilemapSpriteFieldIdMaterial, material);
	Schema_AddEntityId(tilemapSprite, shovelerWorkerSchemaTilemapSpriteFieldIdTilemap, tilemap);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateSpriteTileComponent(
	Worker_EntityId position,
	ShovelerCoordinateMapping positionMappingX,
	ShovelerCoordinateMapping positionMappingY,
	bool enableCollider,
	Worker_EntityId canvas,
	int layer,
	ShovelerVector2 size,
	Worker_EntityId tileSprite)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdSprite;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *sprite = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdPosition, position);
	Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingX, positionMappingX);
	Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingY, positionMappingY);
	Schema_AddBool(sprite, shovelerWorkerSchemaSpriteFieldIdEnableCollider, enableCollider);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdCanvas, canvas);
	Schema_AddInt32(sprite, shovelerWorkerSchemaSpriteFieldIdLayer, layer);
	Schema_Object *sizeObject = Schema_AddObject(sprite, shovelerWorkerSchemaSpriteFieldIdSize);
	Schema_AddFloat(sizeObject, shovelerWorkerSchemaVector2FieldIdX, size.values[0]);
	Schema_AddFloat(sizeObject, shovelerWorkerSchemaVector2FieldIdY, size.values[1]);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdTileSprite, tileSprite);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateSpriteTilemapComponent(
	Worker_EntityId position,
	ShovelerCoordinateMapping positionMappingX,
	ShovelerCoordinateMapping positionMappingY,
	bool enableCollider,
	Worker_EntityId canvas,
	int layer,
	ShovelerVector2 size,
	Worker_EntityId tilemapSprite)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdSprite;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *sprite = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdPosition, position);
	Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingX, positionMappingX);
	Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingY, positionMappingY);
	Schema_AddBool(sprite, shovelerWorkerSchemaSpriteFieldIdEnableCollider, enableCollider);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdCanvas, canvas);
	Schema_AddInt32(sprite, shovelerWorkerSchemaSpriteFieldIdLayer, layer);
	Schema_Object *sizeObject = Schema_AddObject(sprite, shovelerWorkerSchemaSpriteFieldIdSize);
	Schema_AddFloat(sizeObject, shovelerWorkerSchemaVector2FieldIdX, size.values[0]);
	Schema_AddFloat(sizeObject, shovelerWorkerSchemaVector2FieldIdY, size.values[1]);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdTilemapSprite, tilemapSprite);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateSpriteTextureComponent(
	Worker_EntityId position,
	ShovelerCoordinateMapping positionMappingX,
	ShovelerCoordinateMapping positionMappingY,
	bool enableCollider,
	Worker_EntityId canvas,
	int layer,
	ShovelerVector2 size,
	Worker_EntityId textureSprite)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdSprite;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *sprite = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdPosition, position);
	Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingX, positionMappingX);
	Schema_AddEnum(sprite, shovelerWorkerSchemaSpriteFieldIdPositionMappingY, positionMappingY);
	Schema_AddBool(sprite, shovelerWorkerSchemaSpriteFieldIdEnableCollider, enableCollider);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdCanvas, canvas);
	Schema_AddInt32(sprite, shovelerWorkerSchemaSpriteFieldIdLayer, layer);
	Schema_Object *sizeObject = Schema_AddObject(sprite, shovelerWorkerSchemaSpriteFieldIdSize);
	Schema_AddFloat(sizeObject, shovelerWorkerSchemaVector2FieldIdX, size.values[0]);
	Schema_AddFloat(sizeObject, shovelerWorkerSchemaVector2FieldIdY, size.values[1]);
	Schema_AddEntityId(sprite, shovelerWorkerSchemaSpriteFieldIdTextureSprite, textureSprite);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateTileSpriteAnimationComponent(
	Worker_EntityId position,
	Worker_EntityId tileSprite,
	ShovelerCoordinateMapping positionMappingX,
	ShovelerCoordinateMapping positionMappingY,
	float moveAmountThreshold)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdTileSpriteAnimation;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *tileSpriteAnimation = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdPosition, position);
	Schema_AddEntityId(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdTileSprite, tileSprite);
	Schema_AddEnum(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdPositionMappingX, positionMappingX);
	Schema_AddEnum(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdPositionMappingY, positionMappingY);
	Schema_AddFloat(tileSpriteAnimation, shovelerWorkerSchemaTileSpriteAnimationFieldIdMoveAmountThreshold, moveAmountThreshold);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateClientComponent(Worker_EntityId position)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdClient;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *client = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(client, shovelerWorkerSchemaClientFieldIdPosition, position);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateClientHeartPingComponent(int64_t lastUpdatedTime)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdClientHeartbeatPing;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *clientHeartbeatPing = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddInt64(clientHeartbeatPing, shovelerWorkerSchemaClientHeartbeatPingFieldIdLastUpdatedTime, lastUpdatedTime);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateClientHeartPongComponent(int64_t lastUpdatedTime)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdClientHeartbeatPong;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *clientHeartbeatPong = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddInt64(clientHeartbeatPong, shovelerWorkerSchemaClientHeartbeatPongFieldIdLastUpdatedTime, lastUpdatedTime);
	return componentData;
}

Worker_ComponentData shovelerWorkerSchemaCreateClientInfoComponent(
	Worker_EntityId workerEntityId,
	float colorHue,
	float colorSaturation)
{
	Worker_ComponentData componentData;
	componentData.component_id = shovelerWorkerSchemaComponentIdClientInfo;
	componentData.schema_type = Schema_CreateComponentData();
	Schema_Object *clientInfo = Schema_GetComponentDataFields(componentData.schema_type);
	Schema_AddEntityId(clientInfo, shovelerWorkerSchemaClientInfoFieldIdWorkerEntityId, workerEntityId);
	Schema_AddFloat(clientInfo, shovelerWorkerSchemaClientInfoFieldIdColorHue, colorHue);
	Schema_AddFloat(clientInfo, shovelerWorkerSchemaClientInfoFieldIdColorSaturation, colorSaturation);
	return componentData;
}

void shovelerWorkerSchemaAddImprobableAuthorityDelegation(Worker_ComponentData *componentData, Worker_ComponentSetId componentSetId, Worker_EntityId partitionId)
{
	assert(componentData->component_id == shovelerWorkerSchemaComponentIdImprobableAuthorityDelegation);
	Schema_Object *authorityDelegations = Schema_GetComponentDataFields(componentData->schema_type);
	Schema_Object *authorityDelegation = Schema_AddObject(authorityDelegations, shovelerWorkerSchemaImprobableAuthorityDelegationDelegations);
	Schema_AddUint32(authorityDelegation, SCHEMA_MAP_KEY_FIELD_ID, componentSetId);
	Schema_AddUint32(authorityDelegation, SCHEMA_MAP_VALUE_FIELD_ID, partitionId);
}

Schema_Object *shovelerWorkerSchemaAddImprobableInterestForComponent(Worker_ComponentData *componentData, Worker_ComponentId componentId)
{
	assert(componentData->component_id == shovelerWorkerSchemaComponentIdImprobableInterest);
	Schema_Object *interest = Schema_GetComponentDataFields(componentData->schema_type);
	Schema_Object *clientComponentInterestEntry = Schema_AddObject(interest, shovelerWorkerSchemaImprobableInterestFieldIdComponentInterest);
	Schema_AddUint32(clientComponentInterestEntry, SCHEMA_MAP_KEY_FIELD_ID, componentId);
	return Schema_AddObject(clientComponentInterestEntry, SCHEMA_MAP_VALUE_FIELD_ID);
}

Schema_Object *shovelerWorkerSchemaAddImprobableInterestComponentQuery(Schema_Object *componentInterest)
{
	return Schema_AddObject(componentInterest, shovelerWorkerSchemaImprobableComponentInterestFieldIdQueries);
}

void shovelerWorkerSchemaSetImprobableInterestQueryEntityIdConstraint(Schema_Object *query, Worker_EntityId entityId)
{
	Schema_Object *constraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);
	Schema_AddEntityId(constraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdEntityIdConstraint, entityId);
}

void shovelerWorkerSchemaSetImprobableInterestQueryComponentConstraint(Schema_Object *query, Worker_ComponentId componentId)
{
	Schema_Object *constraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);
	Schema_AddEntityId(constraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdComponentConstraint, componentId);
}

void shovelerWorkerSchemaSetImprobableInterestQueryRelativeBoxConstraint(Schema_Object *query, double edgeLengthX, double edgeLengthY, double edgeLengthZ)
{
	Schema_Object *queryConstraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);
	Schema_Object *relativeBoxConstraint = Schema_AddObject(queryConstraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdRelativeBoxConstraint);
	Schema_Object *relativeBoxConstraintEdgeLength = Schema_AddObject(relativeBoxConstraint, shovelerWorkerSchemaImprobableComponentInterestRelativeBoxConstraintFieldIdEdgeLength);
	Schema_AddDouble(relativeBoxConstraintEdgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdX, edgeLengthX);
	Schema_AddDouble(relativeBoxConstraintEdgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdY, edgeLengthY);
	Schema_AddDouble(relativeBoxConstraintEdgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdZ, edgeLengthZ);
}

void shovelerWorkerSchemaSetImprobableInterestQueryRelativeSphereConstraint(Schema_Object *query, double radius)
{
	Schema_Object *queryConstraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);
	Schema_Object *relativeSphereConstraint = Schema_AddObject(queryConstraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdRelativeSphereConstraint);
	Schema_AddDouble(relativeSphereConstraint, shovelerWorkerSchemaImprobableComponentInterestRelativeSphereConstraintFieldIdRadius, radius);
}

void shovelerWorkerSchemaSetImprobableInterestQueryFullSnapshotResult(Schema_Object *query)
{
	Schema_AddBool(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdFullSnapshotResult, true);
}

void shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(Schema_Object *query, Worker_ComponentId componentId)
{
	Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, componentId);
}
