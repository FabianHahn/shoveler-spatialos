#ifndef SHOVELER_SEEDER_TILES_MAP_H
#define SHOVELER_SEEDER_TILES_MAP_H

#include <string>

#include <improbable/standard_library.h>
#include <improbable/worker.h>

extern "C" {
#include <shoveler/types.h>
}

struct TilesData {
	std::string tilesetColumns;
	std::string tilesetRows;
	std::string tilesetIds;
	std::string tilesetColliders;
};

struct ChunkData {
	ShovelerVector2 position;
	TilesData backgroundTiles;
	TilesData foregroundTiles;
};

worker::List<ChunkData> generateMapChunks(int chunkSize);

#endif
