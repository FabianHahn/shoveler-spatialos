#ifndef SHOVELER_SEEDER_TILES_MAP_H
#define SHOVELER_SEEDER_TILES_MAP_H

#include <glib.h>
#include <shoveler/types.h>

typedef struct {
	GString *tilesetColumns;
	GString *tilesetRows;
	GString *tilesetIds;
	GString *tilesetColliders;
} TilesData;

typedef struct {
	ShovelerVector2 position;
	TilesData backgroundTiles;
	TilesData foregroundTiles;
} ChunkData;

/**
 * Returns a list of ChunkData structs.
 *
 * The list and the chunk data structs must be freed by the caller.
 */
GQueue *generateMapChunks(int chunkSize);
void freeMapChunks(GQueue *mapChunks);

#endif
