#ifndef SHOVELER_WORKER_COMMON_SCHEMA_H
#define SHOVELER_WORKER_COMMON_SCHEMA_H

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

const char *shovelerWorkerSchemaResolveSpecialComponentId(int componentId);

#endif
