#include <cstdlib>

#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

extern "C" {
#include <glib.h>

#include <shoveler/image/png.h>
#include <shoveler/resources/image_png.h>
#include <shoveler/constants.h>
#include <shoveler/file.h>
#include <shoveler/log.h>
}

#include "tiles/map.h"
#include "tiles/tileset.h"

using improbable::ComponentInterest;
using improbable::Coordinates;
using improbable::EntityAcl;
using improbable::Interest;
using improbable::InterestData;
using improbable::Metadata;
using improbable::Persistence;
using ImprobablePosition = improbable::Position;
using improbable::WorkerAttributeSet;
using improbable::WorkerRequirementSet;
using shoveler::Bootstrap;
using shoveler::Canvas;
using shoveler::Client;
using shoveler::CoordinateMapping;
using shoveler::Drawable;
using shoveler::DrawableType;
using shoveler::Image;
using shoveler::ImageFormat;
using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Model;
using shoveler::PolygonMode;
using shoveler::Position;
using shoveler::PositionType;
using shoveler::Resource;
using shoveler::Sampler;
using shoveler::Sprite;
using shoveler::Texture;
using shoveler::TextureType;
using shoveler::Tilemap;
using shoveler::TilemapColliders;
using shoveler::TilemapSprite;
using shoveler::TilemapTiles;
using shoveler::Tileset;
using shoveler::TileSprite;
using worker::ComponentRegistry;
using worker::Components;
using worker::Entity;
using worker::EntityId;
using worker::List;
using worker::Map;
using worker::Option;
using worker::Result;
using worker::SnapshotOutputStream;
using worker::StreamErrorCode;

using Query = ComponentInterest::Query;
using QueryConstraint = ComponentInterest::QueryConstraint;

static GString *getImageData(ShovelerImage *image);

static const unsigned int chunkSize = 10;

int main(int argc, char **argv) {
	shovelerLogInit("ShovelCrest/workers/cmake/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);

	if (argc != 10) {
		shovelerLogError("Usage:\n\t%s <tileset png> <tileset columns> <tileset rows> <character png> <character2 png> <character3 png> <character4 png> <character shift amount> <seed snapshot file>", argv[0]);
		return EXIT_FAILURE;
	}

	std::string tilesetPngFilename(argv[1]);
	int tilesetPngColumns = atoi(argv[2]);
	int tilesetPngRows = atoi(argv[3]);
	std::string characterPngFilename(argv[4]);
	std::string character2PngFilename(argv[5]);
	std::string character3PngFilename(argv[6]);
	std::string character4PngFilename(argv[7]);
	int characterShiftAmount = atoi(argv[8]);
	std::string snapshotFilename(argv[9]);

	const ComponentRegistry& components = Components<
		Bootstrap,
		Canvas,
		Client,
		Drawable,
		EntityAcl,
		Image,
		ImprobablePosition,
		Interest,
		Metadata,
		Material,
		Model,
		Persistence,
		Position,
		Resource,
		Sampler,
		Sprite,
		Texture,
		Tilemap,
		TilemapColliders,
		TilemapSprite,
		TilemapTiles,
		Tileset,
		TileSprite>{};

	WorkerAttributeSet clientAttributeSet({"client"});
	WorkerAttributeSet serverAttributeSet({"server"});
	WorkerRequirementSet clientRequirementSet({clientAttributeSet});
	WorkerRequirementSet serverRequirementSet({serverAttributeSet});
	WorkerRequirementSet clientOrServerRequirementSet({clientAttributeSet, serverAttributeSet});

	worker::Map<std::uint32_t, WorkerRequirementSet> resourceToServerAclMap;
	resourceToServerAclMap.insert({{Resource::ComponentId, serverRequirementSet}});

	std::unordered_map<EntityId, Entity> entities;

	Entity bootstrapEntity;
	bootstrapEntity.Add<Metadata>({"bootstrap"});
	bootstrapEntity.Add<Persistence>({});
	bootstrapEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	bootstrapEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	bootstrapEntity.Add<Bootstrap>({});
	Map<std::uint32_t, WorkerRequirementSet> bootstrapComponentAclMap;
	bootstrapComponentAclMap.insert({{Bootstrap::ComponentId, serverRequirementSet}});
	bootstrapEntity.Add<EntityAcl>({clientOrServerRequirementSet, bootstrapComponentAclMap});
	Query query;
	QueryConstraint queryConstraint;
	queryConstraint.set_component_constraint(Client::ComponentId);
	query.set_constraint(queryConstraint);
	query.set_full_snapshot_result({true});
	ComponentInterest componentInterest;
	componentInterest.set_queries({query});
	InterestData interestData;
	interestData.component_interest()[Bootstrap::ComponentId] = componentInterest;
	bootstrapEntity.Add<Interest>(interestData);
	entities[1] = bootstrapEntity;

	Entity quadDrawableEntity;
	quadDrawableEntity.Add<Metadata>({"drawable"});
	quadDrawableEntity.Add<Persistence>({});
	quadDrawableEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	quadDrawableEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	quadDrawableEntity.Add<Drawable>({DrawableType::QUAD, {}, {}});
	quadDrawableEntity.Add<EntityAcl>({clientOrServerRequirementSet, {}});
	entities[2] = quadDrawableEntity;

	ShovelerImage *tilesetImage;
	int tilesetColumns;
	int tilesetRows;
	createTileset(&tilesetImage, &tilesetColumns, &tilesetRows);
	GString *imageData = getImageData(tilesetImage);
	shovelerImageFree(tilesetImage);
	Entity tilesetEntity;
	tilesetEntity.Add<Metadata>({"tileset"});
	tilesetEntity.Add<Persistence>({});
	tilesetEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	tilesetEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	tilesetEntity.Add<Resource>({std::string{imageData->str, imageData->len}});
	tilesetEntity.Add<Image>({ImageFormat::PNG, 3});
	tilesetEntity.Add<Sampler>({true, false, true});
	tilesetEntity.Add<Texture>({TextureType::IMAGE, {3}, {}, {}});
	tilesetEntity.Add<Tileset>({3, tilesetColumns, tilesetRows, 1});
	tilesetEntity.Add<EntityAcl>({clientOrServerRequirementSet, resourceToServerAclMap});
	g_string_free(imageData, true);
	entities[3] = tilesetEntity;

	ShovelerImage *tilesetPngImage = shovelerImagePngReadFile(tilesetPngFilename.c_str());
	GString *tilesetPngData = getImageData(tilesetPngImage);
	shovelerImageFree(tilesetPngImage);
	Entity tilesetPngEntity;
	tilesetPngEntity.Add<Metadata>({"tileset"});
	tilesetPngEntity.Add<Persistence>({});
	tilesetPngEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	tilesetPngEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	tilesetPngEntity.Add<Resource>({std::string{tilesetPngData->str, tilesetPngData->len}});
	tilesetPngEntity.Add<Image>({ImageFormat::PNG, 4});
	tilesetPngEntity.Add<Sampler>({true, false, true});
	tilesetPngEntity.Add<Texture>({TextureType::IMAGE, {4}, {}, {}});
	tilesetPngEntity.Add<Tileset>({4, tilesetPngColumns, tilesetPngRows, 1});
	tilesetPngEntity.Add<EntityAcl>({clientOrServerRequirementSet, resourceToServerAclMap});
	g_string_free(tilesetPngData, true);
	entities[4] = tilesetPngEntity;

	ShovelerImage *characterPngImage = shovelerImagePngReadFile(characterPngFilename.c_str());
	ShovelerImage *characterAnimationTilesetImage = shovelerImageCreateAnimationTileset(characterPngImage, characterShiftAmount);
	GString *characterAnimationTilesetPngData = getImageData(characterAnimationTilesetImage);
	shovelerImageFree(characterPngImage);
	shovelerImageFree(characterAnimationTilesetImage);
	Entity characterAnimationTilesetEntity;
	characterAnimationTilesetEntity.Add<Metadata>({"tileset"});
	characterAnimationTilesetEntity.Add<Persistence>({});
	characterAnimationTilesetEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	characterAnimationTilesetEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	characterAnimationTilesetEntity.Add<Resource>({std::string{characterAnimationTilesetPngData->str, characterAnimationTilesetPngData->len}});
	characterAnimationTilesetEntity.Add<Image>({ImageFormat::PNG, 5});
	characterAnimationTilesetEntity.Add<Sampler>({true, false, true});
	characterAnimationTilesetEntity.Add<Texture>({TextureType::IMAGE, {5}, {}, {}});
	characterAnimationTilesetEntity.Add<Tileset>({5, 4, 3, 1});
	characterAnimationTilesetEntity.Add<EntityAcl>({clientOrServerRequirementSet, resourceToServerAclMap});
	g_string_free(characterAnimationTilesetPngData, true);
	entities[5] = characterAnimationTilesetEntity;

	ShovelerImage *character2PngImage = shovelerImagePngReadFile(character2PngFilename.c_str());
	ShovelerImage *character2AnimationTilesetImage = shovelerImageCreateAnimationTileset(character2PngImage, characterShiftAmount);
	GString *character2AnimationTilesetPngData = getImageData(character2AnimationTilesetImage);
	shovelerImageFree(character2PngImage);
	shovelerImageFree(character2AnimationTilesetImage);
	Entity character2AnimationTilesetEntity;
	character2AnimationTilesetEntity.Add<Metadata>({"tileset"});
	character2AnimationTilesetEntity.Add<Persistence>({});
	character2AnimationTilesetEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	character2AnimationTilesetEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	character2AnimationTilesetEntity.Add<Resource>({std::string{character2AnimationTilesetPngData->str, character2AnimationTilesetPngData->len}});
	character2AnimationTilesetEntity.Add<Image>({ImageFormat::PNG, 6});
	character2AnimationTilesetEntity.Add<Sampler>({true, false, true});
	character2AnimationTilesetEntity.Add<Texture>({TextureType::IMAGE, {6}, {}, {}});
	character2AnimationTilesetEntity.Add<Tileset>({6, 4, 3, 1});
	character2AnimationTilesetEntity.Add<EntityAcl>({clientOrServerRequirementSet, resourceToServerAclMap});
	g_string_free(character2AnimationTilesetPngData, true);
	entities[6] = character2AnimationTilesetEntity;

	ShovelerImage *character3PngImage = shovelerImagePngReadFile(character3PngFilename.c_str());
	ShovelerImage *character3AnimationTilesetImage = shovelerImageCreateAnimationTileset(character3PngImage, characterShiftAmount);
	GString *character3AnimationTilesetPngData = getImageData(character3AnimationTilesetImage);
	shovelerImageFree(character3PngImage);
	shovelerImageFree(character3AnimationTilesetImage);
	Entity character3AnimationTilesetEntity;
	character3AnimationTilesetEntity.Add<Metadata>({"tileset"});
	character3AnimationTilesetEntity.Add<Persistence>({});
	character3AnimationTilesetEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	character3AnimationTilesetEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	character3AnimationTilesetEntity.Add<Resource>({std::string{character3AnimationTilesetPngData->str, character3AnimationTilesetPngData->len}});
	character3AnimationTilesetEntity.Add<Image>({ImageFormat::PNG, 7});
	character3AnimationTilesetEntity.Add<Sampler>({true, false, true});
	character3AnimationTilesetEntity.Add<Texture>({TextureType::IMAGE, {7}, {}, {}});
	character3AnimationTilesetEntity.Add<Tileset>({7, 4, 3, 1});
	character3AnimationTilesetEntity.Add<EntityAcl>({clientOrServerRequirementSet, resourceToServerAclMap});
	g_string_free(character3AnimationTilesetPngData, true);
	entities[7] = character3AnimationTilesetEntity;

	ShovelerImage *character4PngImage = shovelerImagePngReadFile(character4PngFilename.c_str());
	ShovelerImage *character4AnimationTilesetImage = shovelerImageCreateAnimationTileset(character4PngImage, characterShiftAmount);
	GString *character4AnimationTilesetPngData = getImageData(character4AnimationTilesetImage);
	shovelerImageFree(character4PngImage);
	shovelerImageFree(character4AnimationTilesetImage);
	Entity character4AnimationTilesetEntity;
	character4AnimationTilesetEntity.Add<Metadata>({"tileset"});
	character4AnimationTilesetEntity.Add<Persistence>({});
	character4AnimationTilesetEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	character4AnimationTilesetEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	character4AnimationTilesetEntity.Add<Resource>({std::string{character4AnimationTilesetPngData->str, character4AnimationTilesetPngData->len}});
	character4AnimationTilesetEntity.Add<Image>({ImageFormat::PNG, 8});
	character4AnimationTilesetEntity.Add<Sampler>({true, false, true});
	character4AnimationTilesetEntity.Add<Texture>({TextureType::IMAGE, {8}, {}, {}});
	character4AnimationTilesetEntity.Add<Tileset>({8, 4, 3, 1});
	character4AnimationTilesetEntity.Add<EntityAcl>({clientOrServerRequirementSet, resourceToServerAclMap});
	g_string_free(character4AnimationTilesetPngData, true);
	entities[8] = character4AnimationTilesetEntity;

	EntityId canvasEntityId = 9;
	Entity canvasEntity;
	canvasEntity.Add<Metadata>({"canvas"});
	canvasEntity.Add<Material>({MaterialType::TILE_SPRITE, {}, {}, {}, {}, {}, {}, {}, {}});
	canvasEntity.Add<Persistence>({});
	canvasEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	canvasEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	canvasEntity.Add<Canvas>({3});
	worker::Map<std::uint32_t, WorkerRequirementSet> canvasComponentAclMap;
	canvasComponentAclMap.insert({{Canvas::ComponentId, serverRequirementSet}});
	canvasEntity.Add<EntityAcl>({clientOrServerRequirementSet, canvasComponentAclMap});
	entities[canvasEntityId] = canvasEntity;

	EntityId tileSpriteMaterialEntityId = 10;
	Entity tileSpriteMaterialEntity;
	tileSpriteMaterialEntity.Add<Metadata>({"material"});
	tileSpriteMaterialEntity.Add<Material>({MaterialType::TILE_SPRITE, {}, {}, {}, {}, {}, {}, {}, {}});
	tileSpriteMaterialEntity.Add<Persistence>({});
	tileSpriteMaterialEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	tileSpriteMaterialEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	tileSpriteMaterialEntity.Add<EntityAcl>({clientOrServerRequirementSet, {}});
	entities[tileSpriteMaterialEntityId] = tileSpriteMaterialEntity;

	EntityId tilemapMaterialEntityId = 11;
	Entity tilemapMaterialEntity;
	tilemapMaterialEntity.Add<Metadata>({"material"});
	tilemapMaterialEntity.Add<Material>({MaterialType::TILEMAP, {}, {}, {}, {}, {}, {}, {}, {}});
	tilemapMaterialEntity.Add<Persistence>({});
	tilemapMaterialEntity.Add<ImprobablePosition>({{-100, -100, -100}});
	tilemapMaterialEntity.Add<Position>({PositionType::ABSOLUTE, {-100, -100, -100}, {}});
	tilemapMaterialEntity.Add<EntityAcl>({clientOrServerRequirementSet, {}});
	entities[tilemapMaterialEntityId] = tilemapMaterialEntity;

	EntityId nextEntityId = 12;

	List<ChunkData> chunks = generateMapChunks(10);
	for(List<ChunkData>::const_iterator iter = chunks.begin(); iter != chunks.end(); ++iter) {
		ChunkData chunk = *iter;

		Coordinates chunkImprobablePosition{(double) chunk.position.values[0], 0.0, (double) chunk.position.values[1]};
		ShovelerVector3 chunkPosition = shovelerVector3(chunk.position.values[0], chunk.position.values[1], 0.0f);

		Entity chunkBackgroundEntity;
		chunkBackgroundEntity.Add<Metadata>({"chunk_background"});
		chunkBackgroundEntity.Add<Persistence>({});
		chunkBackgroundEntity.Add<ImprobablePosition>({chunkImprobablePosition});
		chunkBackgroundEntity.Add<Position>({PositionType::ABSOLUTE, {chunkPosition.values[0], chunkPosition.values[1], chunkPosition.values[2]}, {}});
		chunkBackgroundEntity.Add<TilemapColliders>({(int32_t) chunkSize, (int32_t) chunkSize, chunk.backgroundTiles.tilesetColliders});
		chunkBackgroundEntity.Add<TilemapTiles>({{}, {chunkSize}, {chunkSize}, {chunk.backgroundTiles.tilesetColumns}, {chunk.backgroundTiles.tilesetRows}, {chunk.backgroundTiles.tilesetIds}});
		chunkBackgroundEntity.Add<Tilemap>({nextEntityId, nextEntityId, {3, 4}});
		chunkBackgroundEntity.Add<TilemapSprite>({tilemapMaterialEntityId, nextEntityId});
		chunkBackgroundEntity.Add<Sprite>({nextEntityId, CoordinateMapping::POSITIVE_X, CoordinateMapping::POSITIVE_Y, true, canvasEntityId, 0, {(float) chunkSize, (float) chunkSize}, {}, {nextEntityId}, {}});
		worker::Map<std::uint32_t, WorkerRequirementSet> chunkBackgroundComponentAclMap;
		chunkBackgroundComponentAclMap.insert({{TilemapTiles::ComponentId, serverRequirementSet}});
		chunkBackgroundEntity.Add<EntityAcl>({clientOrServerRequirementSet, chunkBackgroundComponentAclMap});
		EntityId backgroundEntityId = nextEntityId;
		entities[backgroundEntityId] = chunkBackgroundEntity;
		nextEntityId++;

		Entity chunkForegroundEntity;
		chunkForegroundEntity.Add<Metadata>({"chunk_foreground"});
		chunkForegroundEntity.Add<Persistence>({});
		chunkForegroundEntity.Add<ImprobablePosition>({chunkImprobablePosition});
		chunkForegroundEntity.Add<Position>({PositionType::ABSOLUTE, {chunkPosition.values[0], chunkPosition.values[1], chunkPosition.values[2]}, {}});
		chunkForegroundEntity.Add<TilemapColliders>({(int32_t) chunkSize, (int32_t) chunkSize, chunk.foregroundTiles.tilesetColliders});
		chunkForegroundEntity.Add<TilemapTiles>({{}, {chunkSize}, {chunkSize}, {chunk.foregroundTiles.tilesetColumns}, {chunk.foregroundTiles.tilesetRows}, {chunk.foregroundTiles.tilesetIds}});
		chunkForegroundEntity.Add<Tilemap>({nextEntityId, nextEntityId, {3, 4}});
		chunkForegroundEntity.Add<TilemapSprite>({tilemapMaterialEntityId, nextEntityId});
		chunkForegroundEntity.Add<Sprite>({nextEntityId, CoordinateMapping::POSITIVE_X, CoordinateMapping::POSITIVE_Y, true, canvasEntityId, 2, {(float) chunkSize, (float) chunkSize}, {}, {nextEntityId}, {}});
		chunkForegroundEntity.Add<EntityAcl>({clientOrServerRequirementSet, {}});
		EntityId foregroundEntityId = nextEntityId;
		entities[foregroundEntityId] = chunkForegroundEntity;
		nextEntityId++;

		Entity chunkEntity;
		chunkEntity.Add<Metadata>({"chunk"});
		chunkEntity.Add<Persistence>({});
		chunkEntity.Add<ImprobablePosition>({chunkImprobablePosition});
		chunkEntity.Add<Position>({PositionType::ABSOLUTE, {chunkPosition.values[0], chunkPosition.values[1], chunkPosition.values[2]}, {}});
		chunkEntity.Add<Material>({MaterialType::CANVAS, {}, {}, {}, {}, {canvasEntityId}, {}, {{chunk.position.values[0], chunk.position.values[1]}}, {{(float) chunkSize, (float) chunkSize}}});
		chunkEntity.Add<Model>({nextEntityId, 2, nextEntityId, {0.0, 0.0, 0.0}, {chunkSize / 2, chunkSize / 2, 1.0}, true, true, false, PolygonMode::FILL});
		chunkEntity.Add<EntityAcl>({clientOrServerRequirementSet, {}});
		entities[nextEntityId] = chunkEntity;
		nextEntityId++;
	}

	Result<SnapshotOutputStream, StreamErrorCode> outputStream = SnapshotOutputStream::Create(components, snapshotFilename);
	if(!outputStream) {
		shovelerLogError("Failed to open snapshot stream: %s", outputStream.GetErrorMessage().c_str());
		return EXIT_FAILURE;
	}

	for(std::unordered_map<EntityId, Entity>::const_iterator iter = entities.begin(); iter != entities.end(); ++iter) {
		Result<worker::None, StreamErrorCode> entityWritten = outputStream->WriteEntity(iter->first, iter->second);
		if(!entityWritten) {
			shovelerLogError("Failed to write entity %lld to snapshot: %s", iter->first, entityWritten.GetErrorMessage().c_str());
			return EXIT_FAILURE;
		}
	}

	shovelerLogInfo("Successfully wrote snapshot with %zu entities.", entities.size());
	return EXIT_SUCCESS;
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
