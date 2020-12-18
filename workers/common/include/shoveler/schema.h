#ifndef SHOVELER_WORKER_COMMON_SCHEMA_H
#define SHOVELER_WORKER_COMMON_SCHEMA_H

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include <shoveler/types.h>

enum {
	shovelerWorkerSchemaComponentIdImprobableEntityAcl = 50,
	shovelerWorkerSchemaComponentIdImprobableMetadata = 53,
	shovelerWorkerSchemaComponentIdImprobablePosition = 54,
	shovelerWorkerSchemaComponentIdImprobablePersistence = 55,
	shovelerWorkerSchemaComponentIdImprobableInterest = 58,
	shovelerWorkerSchemaComponentIdImprobableSystem = 59,
	shovelerWorkerSchemaComponentIdImprobableWorker = 60,
	shovelerWorkerSchemaComponentIdImprobablePlayerClient = 61,
	shovelerWorkerSchemaComponentIdPosition = 5454,
	shovelerWorkerSchemaComponentIdBootstrap = 1334,
	shovelerWorkerSchemaComponentIdClient = 1335,
	shovelerWorkerSchemaComponentIdClientInfo = 133742,
	shovelerWorkerSchemaComponentIdClientHeartbeatPing = 13351,
	shovelerWorkerSchemaComponentIdClientHeartbeatPong = 13352,
	shovelerWorkerSchemaComponentIdResource = 1337,
	shovelerWorkerSchemaComponentIdImage = 13377,
	shovelerWorkerSchemaComponentIdTexture = 1338,
	shovelerWorkerSchemaComponentIdSampler = 13381,
	shovelerWorkerSchemaComponentIdTileset = 1339,
	shovelerWorkerSchemaComponentIdTilemapTiles = 1340,
	shovelerWorkerSchemaComponentIdTilemapColliders = 134132,
	shovelerWorkerSchemaComponentIdTilemap = 1341,
	shovelerWorkerSchemaComponentIdSprite = 13422,
	shovelerWorkerSchemaComponentIdTileSprite = 1342,
	shovelerWorkerSchemaComponentIdTilemapSprite = 13421,
	shovelerWorkerSchemaComponentIdTextureSprite = 13423,
	shovelerWorkerSchemaComponentIdTileSpriteAnimation = 1343,
	shovelerWorkerSchemaComponentIdCanvas = 1344,
	shovelerWorkerSchemaComponentIdDrawable = 1346,
	shovelerWorkerSchemaComponentIdMaterial = 1347,
	shovelerWorkerSchemaComponentIdModel = 1348,
	shovelerWorkerSchemaComponentIdLight = 1349,
	shovelerWorkerSchemaComponentIdFont = 1350,
	shovelerWorkerSchemaComponentIdFontAtlas = 1351,
	shovelerWorkerSchemaComponentIdFontAtlasTexture = 1352,
	shovelerWorkerSchemaComponentIdTextTextureRenderer = 1353,
};

enum {
	shovelerWorkerSchemaClientHeartbeatPingFieldIdLastUpdatedTime = 1,
};

enum {
	shovelerWorkerSchemaClientHeartbeatPongFieldIdLastUpdatedTime = 1,
};

enum {
	shovelerWorkerSchemaBootstrapCommandIdCreateClientEntity = 1,
	shovelerWorkerSchemaBootstrapCommandIdClientSpawnCube = 2,
	shovelerWorkerSchemaBootstrapCommandIdDigHole = 3,
	shovelerWorkerSchemaBootstrapCommandIdUpdateResource = 4,
};

enum {
	shovelerWorkerSchemaCreateClientEntityRequestFieldIdStartingChunkRegion = 1,
};

enum {
	shovelerWorkerSchemaUpdateResourceRequestFieldIdResource = 1,
	shovelerWorkerSchemaUpdateResourceRequestFieldIdContent = 2,
};

enum {
	shovelerWorkerSchemaClientSpawnCubeRequestFieldIdClient = 1,
	shovelerWorkerSchemaClientSpawnCubeRequestFieldIdPosition = 2,
	shovelerWorkerSchemaClientSpawnCubeRequestFieldIdDirection = 3,
	shovelerWorkerSchemaClientSpawnCubeRequestFieldIdRotation = 4,
};

enum {
	shovelerWorkerSchemaDigHoleRequestFieldIdClient = 1,
	shovelerWorkerSchemaDigHoleRequestFieldIdPosition = 2,
};


enum {
	shovelerWorkerSchemaChunkRegionFieldIdMinX = 1,
	shovelerWorkerSchemaChunkRegionFieldIdMaxX = 2,
	shovelerWorkerSchemaChunkRegionFieldIdSizeX = 3,
	shovelerWorkerSchemaChunkRegionFieldIdSizeZ = 4,
};

enum {
	shovelerWorkerSchemaImprobableMetadataFieldIdEntityType = 1,
};

enum {
	shovelerWorkerSchemaImprobablePositionFieldIdCoords = 1,
};

enum {
	shovelerWorkerSchemaImprobableCoordinatesFieldIdX = 1,
	shovelerWorkerSchemaImprobableCoordinatesFieldIdY = 2,
	shovelerWorkerSchemaImprobableCoordinatesFieldIdZ = 3,
};

enum {
	shovelerWorkerSchemaPositionFieldIdPositionType = 1,
	shovelerWorkerSchemaPositionFieldIdCoordinates = 2,
	shovelerWorkerSchemaPositionFieldIdRelativeParentPosition = 3,
};

enum {
	shovelerWorkerSchemaVector2FieldIdX = 1,
	shovelerWorkerSchemaVector2FieldIdY = 2,
};

enum {
	shovelerWorkerSchemaVector3FieldIdX = 1,
	shovelerWorkerSchemaVector3FieldIdY = 2,
	shovelerWorkerSchemaVector3FieldIdZ = 3,
};

enum {
	shovelerWorkerSchemaVector4FieldIdX = 1,
	shovelerWorkerSchemaVector4FieldIdY = 2,
	shovelerWorkerSchemaVector4FieldIdZ = 3,
	shovelerWorkerSchemaVector4FieldIdW = 4,
};

enum {
	shovelerWorkerSchemaImprobableInterestFieldIdComponentInterest = 1,
};

enum {
	shovelerWorkerSchemaImprobableComponentInterestFieldIdQueries = 1,
};

enum {
	shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint = 1,
	shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdFullSnapshotResult = 2,
	shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId = 3,
	shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdFrequency = 4,
};

enum {
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdSphereConstraint = 1,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdCylinderConstraint = 2,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdBoxConstraint = 3,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdRelativeSphereConstraint = 4,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdRelativeCylinderConstraint = 5,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdRelativeBoxConstraint = 6,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdEntityIdConstraint = 7,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdComponentConstraint = 8,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdAndConstraint = 9,
	shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdOrConstraint = 10,
};

enum {
	shovelerWorkerSchemaImprobableComponentInterestBoxConstraintFieldIdCenter = 1,
	shovelerWorkerSchemaImprobableComponentInterestBoxConstraintFieldIdEdgeLength = 2,
};

enum {
	shovelerWorkerSchemaImprobableComponentInterestRelativeSphereConstraintFieldIdRadius = 1,
};

enum {
	shovelerWorkerSchemaImprobableComponentInterestRelativeBoxConstraintFieldIdEdgeLength = 1,
};

enum {
	shovelerWorkerSchemaImprobableEdgeLengthFieldIdX = 1,
	shovelerWorkerSchemaImprobableEdgeLengthFieldIdY = 2,
	shovelerWorkerSchemaImprobableEdgeLengthFieldIdZ = 3,
};

enum {
	shovelerWorkerSchemaImprobableEntityAclFieldIdReadAcl = 1,
	shovelerWorkerSchemaImprobableEntityAclFieldIdComponentWriteAcl = 2,
};

enum {
	shovelerWorkerSchemaImprobableWorkerRequirementSetFieldIdAttributeSet = 1,
};

enum {
	shovelerWorkerSchemaImprobableWorkerAttributeSetFieldIdAttribute = 1,
};

enum {
	shovelerWorkerSchemaSpriteFieldIdPosition = 1,
	shovelerWorkerSchemaSpriteFieldIdPositionMappingX = 2,
	shovelerWorkerSchemaSpriteFieldIdPositionMappingY = 3,
	shovelerWorkerSchemaSpriteFieldIdEnableCollider = 4,
	shovelerWorkerSchemaSpriteFieldIdCanvas = 5,
	shovelerWorkerSchemaSpriteFieldIdLayer = 6,
	shovelerWorkerSchemaSpriteFieldIdSize = 7,
	shovelerWorkerSchemaSpriteFieldIdTileSprite = 9,
	shovelerWorkerSchemaSpriteFieldIdTilemapSprite = 10,
	shovelerWorkerSchemaSpriteFieldIdTextureSprite = 11,
};

enum {
	shovelerWorkerSchemaTileSpriteFieldIdMaterial = 1,
	shovelerWorkerSchemaTileSpriteFieldIdTileset = 2,
	shovelerWorkerSchemaTileSpriteFieldIdTilesetColumn = 3,
	shovelerWorkerSchemaTileSpriteFieldIdTilesetRow = 4,
};

enum {
	shovelerWorkerSchemaTileSpriteAnimationFieldIdPosition = 1,
	shovelerWorkerSchemaTileSpriteAnimationFieldIdTileSprite = 2,
	shovelerWorkerSchemaTileSpriteAnimationFieldIdPositionMappingX = 3,
	shovelerWorkerSchemaTileSpriteAnimationFieldIdPositionMappingY = 4,
	shovelerWorkerSchemaTileSpriteAnimationFieldIdMoveAmountThreshold = 5,
};

enum {
	shovelerWorkerSchemaMaterialFieldIdType = 1,
	shovelerWorkerSchemaMaterialFieldIdTextureType = 2,
	shovelerWorkerSchemaMaterialFieldIdTextureSpriteType = 3,
	shovelerWorkerSchemaMaterialFieldIdTexture = 4,
	shovelerWorkerSchemaMaterialFieldIdTextureSampler = 5,
	shovelerWorkerSchemaMaterialFieldIdTilemap = 6,
	shovelerWorkerSchemaMaterialFieldIdCanvas = 7,
	shovelerWorkerSchemaMaterialFieldIdColor = 8,
	shovelerWorkerSchemaMaterialFieldIdCanvasRegionPosition = 9,
	shovelerWorkerSchemaMaterialFieldIdCanvasRegionSize = 10,
};

enum {
	shovelerWorkerSchemaMaterialTypeColor = 0,
	shovelerWorkerSchemaMaterialTypeTexture = 1,
	shovelerWorkerSchemaMaterialTypeParticle = 2,
	shovelerWorkerSchemaMaterialTypeTilemap = 3,
	shovelerWorkerSchemaMaterialTypeCanvas = 4,
	shovelerWorkerSchemaMaterialTypeTextureSprite = 5,
	shovelerWorkerSchemaMaterialTypeTileSprite = 6,
	shovelerWorkerSchemaMaterialTypeText = 7,
};

enum {
	shovelerWorkerSchemaTextureMaterialTypeDepth = 0,
	shovelerWorkerSchemaTextureMaterialTypeAlphaMask = 1,
	shovelerWorkerSchemaTextureMaterialTypeAlbedo = 2,
	shovelerWorkerSchemaTextureMaterialTypePhong = 3,
};

enum {
	shovelerWorkerSchemaTextureSpriteMaterialTypeDepth = 0,
	shovelerWorkerSchemaTextureSpriteMaterialTypeAlphaMask = 1,
	shovelerWorkerSchemaTextureSpriteMaterialTypeAlbedo = 2,
};

enum {
	shovelerWorkerSchemaModelFieldIdPosition = 1,
	shovelerWorkerSchemaModelFieldIdDrawable = 2,
	shovelerWorkerSchemaModelFieldIdMaterial = 3,
	shovelerWorkerSchemaModelFieldIdRotation = 4,
	shovelerWorkerSchemaModelFieldIdScale = 5,
	shovelerWorkerSchemaModelFieldIdVisible = 6,
	shovelerWorkerSchemaModelFieldIdEmitter = 7,
	shovelerWorkerSchemaModelFieldIdCastsShadow = 8,
	shovelerWorkerSchemaModelFieldIdPolygonMode = 9,
};

enum {
	shovelerWorkerSchemaPolygonModePoint = 0,
	shovelerWorkerSchemaPolygonModeLine = 1,
	shovelerWorkerSchemaPolygonModeFill = 2,
};

enum {
	shovelerWorkerSchemaLightFieldIdPosition = 1,
	shovelerWorkerSchemaLightFieldIdType = 2,
	shovelerWorkerSchemaLightFieldIdWidth = 3,
	shovelerWorkerSchemaLightFieldIdHeight = 4,
	shovelerWorkerSchemaLightFieldIdSamples = 5,
	shovelerWorkerSchemaLightFieldIdAmbientFactor = 6,
	shovelerWorkerSchemaLightFieldIdExponentialFactor = 7,
	shovelerWorkerSchemaLightFieldIdColor = 8,
};

enum {
	shovelerWorkerSchemaLightTypeSpot = 0,
	shovelerWorkerSchemaLightTypePoint = 1,
};

enum {
	shovelerWorkerSchemaClientInfoFieldIdWorkerId = 1,
	shovelerWorkerSchemaClientInfoFieldIdColorHue = 2,
	shovelerWorkerSchemaClientInfoFieldIdColorSaturation = 3,
};

enum {
	shovelerWorkerSchemaClientFieldIdPosition = 1,
	shovelerWorkerSchemaClientFieldIdModel = 2,
};

enum {
	shovelerWorkerSchemaTilemapTilesFieldIdImage = 1,
	shovelerWorkerSchemaTilemapTilesFieldIdNumColumns = 2,
	shovelerWorkerSchemaTilemapTilesFieldIdNumRows = 3,
	shovelerWorkerSchemaTilemapTilesFieldIdTilesetColumns = 4,
	shovelerWorkerSchemaTilemapTilesFieldIdTilesetRows = 5,
	shovelerWorkerSchemaTilemapTilesFieldIdTilesetIds = 6,
};

enum {
	shovelerWorkerSchemaResourceFieldIdBuffer = 1,
};

enum {
	shovelerWorkerSchemaImageFieldIdFormat = 1,
	shovelerWorkerSchemaImageFieldIdResource = 2,
};

enum {
	shovelerWorkerSchemaImageFormatPng = 0,
	shovelerWorkerSchemaImageFormatPpm = 1,
};

enum {
	shovelerWorkerSchemaDrawableFieldIdType = 1,
	shovelerWorkerSchemaDrawableFieldIdTilesWidth = 2,
	shovelerWorkerSchemaDrawableFieldIdTilesHeight = 3,
};

enum {
	shovelerWorkerSchemaDrawableTypeCube = 0,
	shovelerWorkerSchemaDrawableTypeQuad = 1,
	shovelerWorkerSchemaDrawableTypePoint = 2,
	shovelerWorkerSchemaDrawableTypeTiles = 3,
};

enum {
	shovelerWorkerSchemaSamplerFieldIdInterpolate = 1,
	shovelerWorkerSchemaSamplerFieldIdUseMipmaps = 2,
	shovelerWorkerSchemaSamplerFieldIdClamp = 3,
};

enum {
	shovelerWorkerSchemaTextureFieldIdType = 1,
	shovelerWorkerSchemaTextureFieldIdImage = 2,
	shovelerWorkerSchemaTextureFieldIdTextTextureRenderer = 3,
	shovelerWorkerSchemaTextureFieldIdText = 4,
};

enum {
	shovelerWorkerSchemaTextureTypeImage = 1,
	shovelerWorkerSchemaTextureTypeText = 2,
};

enum {
	shovelerWorkerSchemaTilesetFieldIdImage = 1,
	shovelerWorkerSchemaTilesetFieldIdNumColumns = 2,
	shovelerWorkerSchemaTilesetFieldIdNumRows = 3,
	shovelerWorkerSchemaTilesetFieldIdPadding = 4,
};

enum {
	shovelerWorkerSchemaCanvasFieldIdNumLayers = 1,
};

enum {
	shovelerWorkerSchemaTilemapCollidersFieldIdNumColumns = 1,
	shovelerWorkerSchemaTilemapCollidersFieldIdNumRows = 2,
	shovelerWorkerSchemaTilemapCollidersFieldIdColliders = 3,
};

enum {
	shovelerWorkerSchemaTilemapFieldIdTiles = 1,
	shovelerWorkerSchemaTilemapFieldIdColliders = 2,
	shovelerWorkerSchemaTilemapFieldIdTilesets = 3,
};

enum {
	shovelerWorkerSchemaTilemapSpriteFieldIdMaterial = 1,
	shovelerWorkerSchemaTilemapSpriteFieldIdTilemap = 2,
};

const char *shovelerWorkerSchemaResolveSpecialComponentId(int componentId);

Worker_ComponentData shovelerWorkerSchemaCreateImprobableMetadataComponent(const char *staticEntityType);
Worker_ComponentData shovelerWorkerSchemaCreateImprobablePersistenceComponent();
Worker_ComponentData shovelerWorkerSchemaCreateImprobablePositionComponent(double x, double y, double z);
Worker_ComponentData shovelerWorkerSchemaCreateImprobableEntityAclComponent();
Worker_ComponentData shovelerWorkerSchemaCreateImprobableInterestComponent();
Worker_ComponentData shovelerWorkerSchemaCreatePositionComponent(ShovelerVector3 positionCoordinates);
Worker_ComponentData shovelerWorkerSchemaCreateBootstrapComponent();
Worker_ComponentData shovelerWorkerSchemaCreateDrawableCubeComponent();
Worker_ComponentData shovelerWorkerSchemaCreateDrawableQuadComponent();
Worker_ComponentData shovelerWorkerSchemaCreateDrawablePointComponent();
Worker_ComponentData shovelerWorkerSchemaCreateDrawableTilesComponent(int tilesWidth, int tilesHeight);
Worker_ComponentData shovelerWorkerSchemaCreateMaterialColorComponent(ShovelerVector4 colorRgba);
Worker_ComponentData shovelerWorkerSchemaCreateMaterialParticleComponent(ShovelerVector4 colorRgba);
Worker_ComponentData shovelerWorkerSchemaCreateMaterialTileSpriteComponent();
Worker_ComponentData shovelerWorkerSchemaCreateMaterialTilemapComponent();
Worker_ComponentData shovelerWorkerSchemaCreateMaterialCanvasComponent(
	Worker_EntityId canvas, ShovelerVector2 regionPosition, ShovelerVector2 regionSize);
Worker_ComponentData shovelerWorkerSchemaCreateModelComponent(
	Worker_EntityId position,
	Worker_EntityId drawable,
	Worker_EntityId material,
	ShovelerVector3 rotation,
	ShovelerVector3 scale,
	bool visible,
	bool emitter,
	bool castsShadow,
	int polygonMode /* TODO: typesafe enum */);
Worker_ComponentData shovelerWorkerSchemaCreateLightComponent(
	Worker_EntityId position,
	int lightType /* TODO: typesafe enum */,
	int width,
	int height,
	int samples,
	float ambientFactor,
	float exponentialFactor,
	ShovelerVector3 color);
Worker_ComponentData shovelerWorkerSchemaCreateResourceComponent(unsigned char *buffer, int bufferSize);
Worker_ComponentData shovelerWorkerSchemaCreateImageComponent(int format  /* TODO: typesafe enum */, Worker_EntityId resource);
Worker_ComponentData shovelerWorkerSchemaCreateSamplerComponent(bool interpolate, bool useMipmaps, bool clamp);
Worker_ComponentData shovelerWorkerSchemaCreateTextureImageComponent(Worker_EntityId image);
Worker_ComponentData shovelerWorkerSchemaCreateTilesetComponent(
	Worker_EntityId image, int numColumns, int numRows, int padding);
Worker_ComponentData shovelerWorkerSchemaCreateCanvasComponent(int numLayers);
Worker_ComponentData shovelerWorkerSchemaCreateTilemapCollidersComponent(
	int numColumns, int numRows, unsigned char *colliders, int collidersSize);
Worker_ComponentData shovelerWorkerSchemaCreateTilemapTilesImageComponent(Worker_EntityId image);
Worker_ComponentData shovelerWorkerSchemaCreateTilemapTilesDirectComponent(
	int numColumns,
	int numRows,
	unsigned char *columns,
	unsigned char *rows,
	unsigned char *ids);
Worker_ComponentData shovelerWorkerSchemaCreateTilemapComponent(
	Worker_EntityId tiles,
	Worker_EntityId colliders,
	Worker_EntityId *tilesets,
	int tilesetsSize);
Worker_ComponentData shovelerWorkerSchemaCreateTilemapSpriteComponent(Worker_EntityId material, Worker_EntityId tilemap);
Worker_ComponentData shovelerWorkerSchemaCreateSpriteTileComponent(
	Worker_EntityId position,
	ShovelerCoordinateMapping positionMappingX,
	ShovelerCoordinateMapping positionMappingY,
	bool enableCollider,
	Worker_EntityId canvas,
	int layer,
	ShovelerVector2 size,
	Worker_EntityId tileSprite);
Worker_ComponentData shovelerWorkerSchemaCreateSpriteTilemapComponent(
	Worker_EntityId position,
	ShovelerCoordinateMapping positionMappingX,
	ShovelerCoordinateMapping positionMappingY,
	bool enableCollider,
	Worker_EntityId canvas,
	int layer,
	ShovelerVector2 size,
	Worker_EntityId tilemapSprite);
Worker_ComponentData shovelerWorkerSchemaCreateSpriteTextureComponent(
	Worker_EntityId position,
	ShovelerCoordinateMapping positionMappingX,
	ShovelerCoordinateMapping positionMappingY,
	bool enableCollider,
	Worker_EntityId canvas,
	int layer,
	ShovelerVector2 size,
	Worker_EntityId textureSprite);

void shovelerWorkerSchemaAddImprobableEntityAclReadStatic(Worker_ComponentData *componentData, const char *staticAttribute);
void shovelerWorkerSchemaAddImprobableEntityAclWriteStatic(Worker_ComponentData *componentData, Worker_ComponentId componentId, const char *staticAttribute);
Schema_Object *shovelerWorkerSchemaAddImprobableInterestForComponent(Worker_ComponentData *componentData, Worker_ComponentId componentId);
Schema_Object *shovelerWorkerSchemaAddImprobableInterestComponentQuery(Schema_Object *componentInterest);
void shovelerWorkerSchemaSetImprobableInterestQueryEntityIdConstraint(Schema_Object *query, Worker_EntityId entityId);
void shovelerWorkerSchemaSetImprobableInterestQueryComponentConstraint(Schema_Object *query, Worker_ComponentId componentId);
void shovelerWorkerSchemaSetImprobableInterestQueryRelativeBoxConstraint(Schema_Object *query, double edgeLengthX, double edgeLengthY, double edgeLengthZ);
void shovelerWorkerSchemaSetImprobableInterestQueryRelativeSphereConstraint(Schema_Object *query, double radius);
void shovelerWorkerSchemaSetImprobableInterestQueryFullSnapshotResult(Schema_Object *query);
void shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(Schema_Object *query, Worker_ComponentId componentId);

#endif
