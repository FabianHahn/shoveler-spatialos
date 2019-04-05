#ifndef SHOVELER_SEEDER_TILES_MAP_H
#define SHOVELER_SEEDER_TILES_MAP_H

#include <improbable/standard_library.h>
#include <improbable/worker.h>
#include <shoveler.h>

struct ChunkData {
	improbable::Coordinates position;
	worker::List<shoveler::TilemapTilesTile> backgroundTiles;
	worker::List<shoveler::TilemapTilesTile> foregroundTiles;
};

worker::List<ChunkData> generateMapChunks(int chunkSize);

#endif
