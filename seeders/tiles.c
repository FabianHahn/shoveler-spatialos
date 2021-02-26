#include <inttypes.h> // PRId64
#include <stdbool.h> // bool
#include <stdlib.h> // EXIT_FAILURE atoi
#include <string.h> // strlen

#include <glib.h>
#include <improbable/c_worker.h>
#include <improbable/c_schema.h>
#include <shoveler/constants.h>
#include <shoveler/file.h>
#include <shoveler/image.h>
#include <shoveler/image/png.h>
#include <shoveler/log.h>
#include <shoveler/schema.h>

#include "map.h"
#include "tileset.h"

static const int chunkSize = 10;
static const int64_t serverPartitionEntityId = 1;

static bool addCharacterAnimationTilesetEntity(Worker_SnapshotOutputStream *snapshotOutputStream, Worker_EntityId *nextEntityId, const char *filename, int shiftAmount);
static bool writeEntity(Worker_SnapshotOutputStream *snapshotOutputStream, Worker_Entity *entity);
static GString *getImageData(ShovelerImage *image);

int main(int argc, char **argv)
{
	shovelerLogInit("shoveler-spatialos/seeders/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);

	if (argc != 10) {
		shovelerLogError("Usage:\n\t%s <tileset png> <tileset columns> <tileset rows> <character png> <character2 png> <character3 png> <character4 png> <character shift amount> <seed snapshot file>", argv[0]);
		return EXIT_FAILURE;
	}

	const char *tilesetPngFilename = argv[1];
	int tilesetPngColumns = atoi(argv[2]);
	int tilesetPngRows = atoi(argv[3]);
	const char *character1PngFilename = argv[4];
	const char *character2PngFilename = argv[5];
	const char *character3PngFilename = argv[6];
	const char *character4PngFilename = argv[7];
	int characterShiftAmount = atoi(argv[8]);
	const char *snapshotFilename = argv[9];

	Worker_ComponentVtable componentVtable = {0};

	Worker_SnapshotParameters snapshotParameters = {0};
	snapshotParameters.snapshot_type = WORKER_SNAPSHOT_TYPE_BINARY;
	snapshotParameters.default_component_vtable = &componentVtable;

	Worker_SnapshotOutputStream *snapshotOutputStream = Worker_SnapshotOutputStream_Create(snapshotFilename, &snapshotParameters);
	if (snapshotOutputStream == NULL) {
		shovelerLogError("Failed to create snapshot output stream to '%s'.", snapshotFilename);
		return EXIT_FAILURE;
	}

	Worker_SnapshotState snapshotState = Worker_SnapshotOutputStream_GetState(snapshotOutputStream);
	if (snapshotState.stream_state != WORKER_STREAM_STATE_GOOD) {
		shovelerLogError("Snapshot output stream not in good state after opening: %s", snapshotState.error_message);
		return EXIT_FAILURE;
	}

	Worker_EntityId nextEntityId = 1;
	{ // bootstrap
		Worker_ComponentData componentData[7] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("bootstrap");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
		componentData[4] = shovelerWorkerSchemaCreateBootstrapComponent();
		componentData[5] = shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent();
		shovelerWorkerSchemaAddImprobableAuthorityDelegation(&componentData[5], shovelerWorkerSchemaComponentSetIdServerBootstrapAuthority, serverPartitionEntityId);
		componentData[6] = shovelerWorkerSchemaCreateImprobableInterestComponent();
		Schema_Object *componentInterest = shovelerWorkerSchemaAddImprobableInterestForComponent(
			&componentData[6], shovelerWorkerSchemaComponentIdBootstrap);
		Schema_Object *query = shovelerWorkerSchemaAddImprobableInterestComponentQuery(componentInterest);
		shovelerWorkerSchemaSetImprobableInterestQueryComponentConstraint(query, shovelerWorkerSchemaComponentIdClient);
		shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(
			query, shovelerWorkerSchemaComponentIdClientHeartbeatPing);
		shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(
			query, shovelerWorkerSchemaComponentIdClientInfo);

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId quadDrawableEntityId = nextEntityId;
	{ // quad drawable
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("drawable");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
		componentData[4] = shovelerWorkerSchemaCreateDrawableQuadComponent();

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId tilesetEntityId = nextEntityId;
	{ // tileset
		ShovelerImage *tilesetImage;
		int tilesetColumns;
		int tilesetRows;
		createTileset(&tilesetImage, &tilesetColumns, &tilesetRows);
		GString *imageData = getImageData(tilesetImage);
		shovelerImageFree(tilesetImage);

		Worker_ComponentData componentData[10] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("tileset");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
		componentData[4] = shovelerWorkerSchemaCreateResourceComponent(
			(unsigned char *) imageData->str, imageData->len);
		componentData[5] = shovelerWorkerSchemaCreateImageComponent(
			shovelerWorkerSchemaImageFormatPng, /* resource */ 0);
		componentData[6] = shovelerWorkerSchemaCreateSamplerComponent(
			/* interpolate */ true, /* useMipmaps */ false, /* clamp */ true);
		componentData[7] = shovelerWorkerSchemaCreateTextureImageComponent(/* image */ 0);
		componentData[8] = shovelerWorkerSchemaCreateTilesetComponent(
			/* image */ 0, tilesetColumns, tilesetRows, /* padding */ 1);
		componentData[9] = shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent();
		shovelerWorkerSchemaAddImprobableAuthorityDelegation(&componentData[9], shovelerWorkerSchemaComponentSetIdServerAssetAuthority, serverPartitionEntityId);

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}

		g_string_free(imageData, true);
	}

	Worker_EntityId tilesetPngEntityId = nextEntityId;
	{ // tileset png
		ShovelerImage *tilesetPngImage = shovelerImagePngReadFile(tilesetPngFilename);
		GString *tilesetPngData = getImageData(tilesetPngImage);
		shovelerImageFree(tilesetPngImage);

		Worker_ComponentData componentData[10] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("tileset");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
		componentData[4] = shovelerWorkerSchemaCreateResourceComponent(
			(unsigned char *) tilesetPngData->str, tilesetPngData->len);
		componentData[5] = shovelerWorkerSchemaCreateImageComponent(
			shovelerWorkerSchemaImageFormatPng, /* resource */ 0);
		componentData[6] = shovelerWorkerSchemaCreateSamplerComponent(
			/* interpolate */ true, /* useMipmaps */ false, /* clamp */ true);
		componentData[7] = shovelerWorkerSchemaCreateTextureImageComponent(/* image */ 0);
		componentData[8] = shovelerWorkerSchemaCreateTilesetComponent(
			/* image */ 0, tilesetPngColumns, tilesetPngRows, /* padding */ 1);
		componentData[9] = shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent();
		shovelerWorkerSchemaAddImprobableAuthorityDelegation(&componentData[9], shovelerWorkerSchemaComponentSetIdServerAssetAuthority, serverPartitionEntityId);

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}

		g_string_free(tilesetPngData, true);
	}

	// character 1 animation tileset
	if (!addCharacterAnimationTilesetEntity(
		snapshotOutputStream, &nextEntityId, character1PngFilename, characterShiftAmount)) {
		return EXIT_FAILURE;
	}

	// character 2 animation tileset
	if (!addCharacterAnimationTilesetEntity(
		snapshotOutputStream, &nextEntityId, character2PngFilename, characterShiftAmount)) {
		return EXIT_FAILURE;
	}

	// character 3 animation tileset
	if (!addCharacterAnimationTilesetEntity(
		snapshotOutputStream, &nextEntityId, character3PngFilename, characterShiftAmount)) {
		return EXIT_FAILURE;
	}

	// character 4 animation tileset
	if (!addCharacterAnimationTilesetEntity(
		snapshotOutputStream, &nextEntityId, character4PngFilename, characterShiftAmount)) {
		return EXIT_FAILURE;
	}

	Worker_EntityId canvasEntityId = nextEntityId;
	{ // canvas
		Worker_ComponentData componentData[6] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("canvas");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
		componentData[4] = shovelerWorkerSchemaCreateMaterialTileSpriteComponent();
		componentData[5] = shovelerWorkerSchemaCreateCanvasComponent(/* numLayers */ 3);

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	{ // tile sprite material
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("material");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
		componentData[4] = shovelerWorkerSchemaCreateMaterialTileSpriteComponent();

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId tilemapMaterialEntityId = nextEntityId;
	{ // tilemap material
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("material");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
		componentData[4] = shovelerWorkerSchemaCreateMaterialTilemapComponent();

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	GQueue *mapChunks = generateMapChunks(chunkSize);
	for(GList *iter = mapChunks->head; iter != NULL; iter = iter->next) {
		ChunkData *chunkData = iter->data;

		double chunkImprobablePositionX = chunkData->position.values[0];
		double chunkImprobablePositionY = 0.0;
		double chunkImprobablePositionZ = chunkData->position.values[1];
		ShovelerVector3 chunkPosition = shovelerVector3(
			chunkData->position.values[0], chunkData->position.values[1], 0.0f);

		{ // chunk background
			Worker_ComponentData componentData[10] = {0};
			Worker_Entity entity;
			entity.entity_id = nextEntityId++;
			entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
			entity.components = componentData;
			componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("chunk_background");
			componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
			componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(
				chunkImprobablePositionX, chunkImprobablePositionY, chunkImprobablePositionZ);
			componentData[3] = shovelerWorkerSchemaCreatePositionComponent(chunkPosition);
			componentData[4] = shovelerWorkerSchemaCreateTilemapCollidersComponent(
				/* numColums */ chunkSize,
				/* numRows */ chunkSize,
				(unsigned char *) chunkData->backgroundTiles.tilesetColliders->str,
				chunkData->backgroundTiles.tilesetColliders->len);
			componentData[5] = shovelerWorkerSchemaCreateTilemapTilesDirectComponent(
				/* numColums */ chunkSize,
				/* numRows */ chunkSize,
				(unsigned char *) chunkData->backgroundTiles.tilesetColumns->str,
				(unsigned char *) chunkData->backgroundTiles.tilesetRows->str,
				(unsigned char *) chunkData->backgroundTiles.tilesetIds->str);
			componentData[6] = shovelerWorkerSchemaCreateTilemapComponent(
				/* tiles */ 0,
				/* collider */ 0,
				/* tilesets */ (Worker_EntityId[]){tilesetEntityId, tilesetPngEntityId},
				/* numTilesets */ 2);
			componentData[7] = shovelerWorkerSchemaCreateTilemapSpriteComponent(
				tilemapMaterialEntityId, /* tilemap */ 0);
			componentData[8] = shovelerWorkerSchemaCreateSpriteTilemapComponent(
				/* position */ 0,
				SHOVELER_COORDINATE_MAPPING_POSITIVE_X,
				SHOVELER_COORDINATE_MAPPING_POSITIVE_Y,
				/* enableCollider */ true,
				canvasEntityId,
				/* layer */ 0,
				/* size */ shovelerVector2((float) chunkSize, (float) chunkSize),
				/* tilemapSprite */ 0);
			componentData[9] = shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent();
			shovelerWorkerSchemaAddImprobableAuthorityDelegation(&componentData[9], shovelerWorkerSchemaComponentSetIdServerAssetAuthority, serverPartitionEntityId);

			if(!writeEntity(snapshotOutputStream, &entity)) {
				return EXIT_FAILURE;
			}
		}

		{ // chunk foreground
			Worker_ComponentData componentData[10] = {0};
			Worker_Entity entity;
			entity.entity_id = nextEntityId++;
			entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
			entity.components = componentData;
			componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("chunk_foreground");
			componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
			componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(
				chunkImprobablePositionX, chunkImprobablePositionY, chunkImprobablePositionZ);
			componentData[3] = shovelerWorkerSchemaCreatePositionComponent(chunkPosition);
			componentData[4] = shovelerWorkerSchemaCreateTilemapCollidersComponent(
				/* numColums */ chunkSize,
				/* numRows */ chunkSize,
				(unsigned char *) chunkData->foregroundTiles.tilesetColliders->str,
				chunkData->foregroundTiles.tilesetColliders->len);
			componentData[5] = shovelerWorkerSchemaCreateTilemapTilesDirectComponent(
				/* numColums */ chunkSize,
				/* numRows */ chunkSize,
				(unsigned char *) chunkData->foregroundTiles.tilesetColumns->str,
				(unsigned char *) chunkData->foregroundTiles.tilesetRows->str,
				(unsigned char *) chunkData->foregroundTiles.tilesetIds->str);
			componentData[6] = shovelerWorkerSchemaCreateTilemapComponent(
				/* tiles */ 0,
				/* collider */ 0,
				/* tilesets */ (Worker_EntityId[]){tilesetEntityId, tilesetPngEntityId},
				/* numTilesets */ 2);
			componentData[7] = shovelerWorkerSchemaCreateTilemapSpriteComponent(
				tilemapMaterialEntityId, /* tilemap */ 0);
			componentData[8] = shovelerWorkerSchemaCreateSpriteTilemapComponent(
				/* position */ 0,
				SHOVELER_COORDINATE_MAPPING_POSITIVE_X,
				SHOVELER_COORDINATE_MAPPING_POSITIVE_Y,
				/* enableCollider */ true,
				canvasEntityId,
				/* layer */ 2,
				/* size */ shovelerVector2((float) chunkSize, (float) chunkSize),
				/* tilemapSprite */ 0);
			componentData[9] = shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent();
			shovelerWorkerSchemaAddImprobableAuthorityDelegation(&componentData[9], shovelerWorkerSchemaComponentSetIdServerAssetAuthority, serverPartitionEntityId);

			if(!writeEntity(snapshotOutputStream, &entity)) {
				return EXIT_FAILURE;
			}
		}

		{ // chunk
			Worker_ComponentData componentData[6] = {0};
			Worker_Entity entity;
			entity.entity_id = nextEntityId++;
			entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
			entity.components = componentData;
			componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("chunk");
			componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
			componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(
				chunkImprobablePositionX, chunkImprobablePositionY, chunkImprobablePositionZ);
			componentData[3] = shovelerWorkerSchemaCreatePositionComponent(chunkPosition);
			componentData[4] = shovelerWorkerSchemaCreateMaterialCanvasComponent(
				canvasEntityId,
				chunkData->position,
				/* regionSize */ shovelerVector2((float) chunkSize, (float) chunkSize));
			componentData[5] = shovelerWorkerSchemaCreateModelComponent(
				/* position */ 0,
				quadDrawableEntityId,
				/* material */ 0,
				/* rotation */ shovelerVector3(0.0f, 0.0f, 0.0f),
				/* scale */ shovelerVector3(0.5f * (float) chunkSize, 0.5f * (float) chunkSize, 1.0f),
				/* visible */ true,
				/* emitter */ true,
				/* castsShadow */ false,
				shovelerWorkerSchemaPolygonModeFill);

			if(!writeEntity(snapshotOutputStream, &entity)) {
				return EXIT_FAILURE;
			}
		}
	}

	freeMapChunks(mapChunks);


	Worker_SnapshotOutputStream_Destroy(snapshotOutputStream);
	shovelerLogInfo("Successfully wrote tiles snapshot with %"PRId64" entities.", nextEntityId - 1);
}

static bool addCharacterAnimationTilesetEntity(Worker_SnapshotOutputStream *snapshotOutputStream, Worker_EntityId *nextEntityId, const char *filename, int shiftAmount)
{
	ShovelerImage *characterPngImage = shovelerImagePngReadFile(filename);
	ShovelerImage *characterAnimationTilesetImage = shovelerImageCreateAnimationTileset(characterPngImage, shiftAmount);
	GString *characterAnimationTilesetPngData = getImageData(characterAnimationTilesetImage);
	shovelerImageFree(characterPngImage);
	shovelerImageFree(characterAnimationTilesetImage);

	Worker_ComponentData componentData[10] = {0};
	Worker_Entity entity;
	entity.entity_id = (*nextEntityId)++;
	entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
	entity.components = componentData;
	componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("tileset");
	componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
	componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(-100.0, -100.0, -100.0);
	componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-100.0f, -100.0f, -100.0f));
	componentData[4] = shovelerWorkerSchemaCreateResourceComponent(
		(unsigned char *) characterAnimationTilesetPngData->str, characterAnimationTilesetPngData->len);
	componentData[5] = shovelerWorkerSchemaCreateImageComponent(
		shovelerWorkerSchemaImageFormatPng, /* resource */ 0);
	componentData[6] = shovelerWorkerSchemaCreateSamplerComponent(
		/* interpolate */ true, /* useMipmaps */ false, /* clamp */ true);
	componentData[7] = shovelerWorkerSchemaCreateTextureImageComponent(/* image */ 0);
	componentData[8] = shovelerWorkerSchemaCreateTilesetComponent(
		/* image */ 0, /* columns */ 4, /* rows */ 3, /* padding */ 1);
	componentData[9] = shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent();
	shovelerWorkerSchemaAddImprobableAuthorityDelegation(&componentData[9], shovelerWorkerSchemaComponentSetIdServerAssetAuthority, serverPartitionEntityId);

	if(!writeEntity(snapshotOutputStream, &entity)) {
		return false;
	}

	g_string_free(characterAnimationTilesetPngData, true);

	return true;
}

static bool writeEntity(Worker_SnapshotOutputStream *snapshotOutputStream, Worker_Entity *entity)
{
	Worker_SnapshotOutputStream_WriteEntity(snapshotOutputStream, entity);
	const char *lastErrorMessage = Worker_SnapshotOutputStream_GetLastWarning(snapshotOutputStream);
	if (lastErrorMessage != NULL) {
		shovelerLogError("Failed to write entity %"PRId64" to snapshot output stream: %s", entity->entity_id, lastErrorMessage);
		return false;
	}

	Worker_SnapshotState snapshotState = Worker_SnapshotOutputStream_GetState(snapshotOutputStream);
	if (snapshotState.stream_state != WORKER_STREAM_STATE_GOOD) {
		shovelerLogError("Snapshot output stream not in good state after writing entity %"PRId64": %s", entity->entity_id, snapshotState.error_message);
		return EXIT_FAILURE;
	}

	return true;
}

static GString *getImageData(ShovelerImage *image)
{
	const char *tempImageFilename = "temp.png";
	shovelerImagePngWriteFile(image, tempImageFilename);

	unsigned char *contents;
	size_t contentsSize;
	shovelerFileRead(tempImageFilename, &contents, &contentsSize);

	GString *data = g_string_new("");
	g_string_append_len(data, (gchar *) contents, contentsSize);
	free(contents);

	return data;
}
