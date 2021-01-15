#include "map.h"

#include <stdlib.h>

static const int halfMapWidth = 100;
static const int halfMapHeight = 100;

static ChunkData *createBlankChunk(int chunkSize, float positionX, float positionZ);
static ChunkData *createStartingAreaBottomLeft(int chunkSize);
static ChunkData *createStartingAreaBottomRight(int chunkSize);
static ChunkData *createStartingAreaTopLeft(int chunkSize);
static ChunkData *createStartingAreaTopRight(int chunkSize);
static ChunkData *createChunk(int x, int y, int chunkSize);

GQueue *generateMapChunks(int chunkSize)
{
	GQueue *chunks = g_queue_new();

	for(int x = -halfMapWidth; x < halfMapWidth; x += chunkSize) {
		for(int z = -halfMapHeight; z < halfMapHeight; z += chunkSize) {
			if(x == -chunkSize && z == -chunkSize) {
				g_queue_push_tail(chunks, createStartingAreaBottomLeft(chunkSize));
			} else if(x == 0 && z == -chunkSize) {
				g_queue_push_tail(chunks, createStartingAreaBottomRight(chunkSize));
			} else if(x == -chunkSize && z == 0) {
				g_queue_push_tail(chunks, createStartingAreaTopLeft(chunkSize));
			} else if(x == 0 && z == 0) {
				g_queue_push_tail(chunks, createStartingAreaTopRight(chunkSize));
			} else {
				g_queue_push_tail(chunks, createChunk(x, z, chunkSize));
			}
		}
	}

	return chunks;
}

void freeMapChunks(GQueue *mapChunks)
{
	for(GList *iter = mapChunks->head; iter != NULL; iter = iter->next) {
		ChunkData *chunk = iter->data;
		g_string_free(chunk->backgroundTiles.tilesetColumns, true);
		g_string_free(chunk->backgroundTiles.tilesetRows, true);
		g_string_free(chunk->backgroundTiles.tilesetIds, true);
		g_string_free(chunk->backgroundTiles.tilesetColliders, true);
		g_string_free(chunk->foregroundTiles.tilesetColumns, true);
		g_string_free(chunk->foregroundTiles.tilesetRows, true);
		g_string_free(chunk->foregroundTiles.tilesetIds, true);
		g_string_free(chunk->foregroundTiles.tilesetColliders, true);
	}

	g_queue_free(mapChunks);
}

static ChunkData *createBlankChunk(int chunkSize, float positionX, float positionZ)
{
	ChunkData *chunk = malloc(sizeof(ChunkData));
	chunk->position = shovelerVector2(positionX, positionZ);
	chunk->backgroundTiles.tilesetColumns = g_string_new("");
	chunk->backgroundTiles.tilesetRows = g_string_new("");
	chunk->backgroundTiles.tilesetIds = g_string_new("");
	chunk->backgroundTiles.tilesetColliders = g_string_new("");
	chunk->foregroundTiles.tilesetColumns = g_string_new("");
	chunk->foregroundTiles.tilesetRows = g_string_new("");
	chunk->foregroundTiles.tilesetIds = g_string_new("");
	chunk->foregroundTiles.tilesetColliders = g_string_new("");
	return chunk;
}

static ChunkData *createStartingAreaBottomLeft(int chunkSize)
{
	ChunkData *chunk = createBlankChunk(chunkSize, (float) -chunkSize / 2.0f, (float) -chunkSize / 2.0f);

	for(int i = 0; i < chunkSize; i++) {
		for(int j = 0; j < chunkSize; j++) {
			char backgroundTileColumn = 0;
			char backgroundTileRow = 0;
			char backgroundTileId = 0;
			char backgroundTileCollider = 0;

			char foregroundTileColumn = 0;
			char foregroundTileRow = 0;
			char foregroundTileId = 0;
			char foregroundTileCollider = 0;

			// fill background with grass
			int grassColumn = rand() % 3;
			int grassRow = rand() % 2;
			backgroundTileColumn = grassColumn;
			backgroundTileRow = grassRow;
			backgroundTileId = 2;
			backgroundTileCollider = false;

			// place boundary rocks
			if((i == 0 || j == 0) && i != chunkSize - 1 && j != chunkSize - 1) {
				backgroundTileColumn = 5;
				backgroundTileRow = 0;
				backgroundTileCollider = true;
			}

			g_string_append_c(chunk->backgroundTiles.tilesetColumns, backgroundTileColumn);
			g_string_append_c(chunk->backgroundTiles.tilesetRows, backgroundTileRow);
			g_string_append_c(chunk->backgroundTiles.tilesetIds, backgroundTileId);
			g_string_append_c(chunk->backgroundTiles.tilesetColliders, backgroundTileCollider);

			g_string_append_c(chunk->foregroundTiles.tilesetColumns, foregroundTileColumn);
			g_string_append_c(chunk->foregroundTiles.tilesetRows, foregroundTileRow);
			g_string_append_c(chunk->foregroundTiles.tilesetIds, foregroundTileId);
			g_string_append_c(chunk->foregroundTiles.tilesetColliders, foregroundTileCollider);
		}
	}

	return chunk;
}

static ChunkData *createStartingAreaBottomRight(int chunkSize)
{
	ChunkData *chunk = createBlankChunk(chunkSize, (float) chunkSize / 2.0f, (float) -chunkSize / 2.0f);

	for(int i = 0; i < chunkSize; i++) {
		for(int j = 0; j < chunkSize; j++) {
			char backgroundTileColumn = 0;
			char backgroundTileRow = 0;
			char backgroundTileId = 0;
			char backgroundTileCollider = 0;

			char foregroundTileColumn = 0;
			char foregroundTileRow = 0;
			char foregroundTileId = 0;
			char foregroundTileCollider = 0;

			foregroundTileColumn = 0;
			foregroundTileRow = 0;
			foregroundTileId = 0;
			foregroundTileCollider = false;

			// fill background with grass
			int grassColumn = rand() % 3;
			int grassRow = rand() % 2;
			backgroundTileColumn = grassColumn;
			backgroundTileRow = grassRow;
			backgroundTileId = 2;
			backgroundTileCollider = false;

			// place boundary rocks
			if((i == 0 || j == chunkSize - 1) && i != chunkSize - 1 && j != 0) {
				backgroundTileColumn = 5;
				backgroundTileRow = 0;
				backgroundTileCollider = true;
			}

			g_string_append_c(chunk->backgroundTiles.tilesetColumns, backgroundTileColumn);
			g_string_append_c(chunk->backgroundTiles.tilesetRows, backgroundTileRow);
			g_string_append_c(chunk->backgroundTiles.tilesetIds, backgroundTileId);
			g_string_append_c(chunk->backgroundTiles.tilesetColliders, backgroundTileCollider);

			g_string_append_c(chunk->foregroundTiles.tilesetColumns, foregroundTileColumn);
			g_string_append_c(chunk->foregroundTiles.tilesetRows, foregroundTileRow);
			g_string_append_c(chunk->foregroundTiles.tilesetIds, foregroundTileId);
			g_string_append_c(chunk->foregroundTiles.tilesetColliders, foregroundTileCollider);
		}
	}

	return chunk;
}

static ChunkData *createStartingAreaTopLeft(int chunkSize)
{
	ChunkData *chunk = createBlankChunk(chunkSize, (float) -chunkSize / 2.0f, (float) chunkSize / 2.0f);

	for(int i = 0; i < chunkSize; i++) {
		for(int j = 0; j < chunkSize; j++) {
			char backgroundTileColumn = 0;
			char backgroundTileRow = 0;
			char backgroundTileId = 0;
			char backgroundTileCollider = 0;

			char foregroundTileColumn = 0;
			char foregroundTileRow = 0;
			char foregroundTileId = 0;
			char foregroundTileCollider = 0;

			foregroundTileColumn = 0;
			foregroundTileRow = 0;
			foregroundTileId = 0;
			foregroundTileCollider = false;

			// fill background with grass
			int grassColumn = rand() % 3;
			int grassRow = rand() % 2;
			backgroundTileColumn = grassColumn;
			backgroundTileRow = grassRow;
			backgroundTileId = 2;
			foregroundTileCollider = false;

			// place boundary rocks
			if((i == chunkSize - 1 || j == 0) && i != 0 && j != chunkSize - 1) {
				backgroundTileColumn = 5;
				backgroundTileRow = 0;
				backgroundTileCollider = true;
			}

			g_string_append_c(chunk->backgroundTiles.tilesetColumns, backgroundTileColumn);
			g_string_append_c(chunk->backgroundTiles.tilesetRows, backgroundTileRow);
			g_string_append_c(chunk->backgroundTiles.tilesetIds, backgroundTileId);
			g_string_append_c(chunk->backgroundTiles.tilesetColliders, backgroundTileCollider);

			g_string_append_c(chunk->foregroundTiles.tilesetColumns, foregroundTileColumn);
			g_string_append_c(chunk->foregroundTiles.tilesetRows, foregroundTileRow);
			g_string_append_c(chunk->foregroundTiles.tilesetIds, foregroundTileId);
			g_string_append_c(chunk->foregroundTiles.tilesetColliders, foregroundTileCollider);
		}
	}

	return chunk;
}

static ChunkData *createStartingAreaTopRight(int chunkSize)
{
	ChunkData *chunk = createBlankChunk(chunkSize, (float) chunkSize / 2.0f, (float) chunkSize / 2.0f);

	for(int i = 0; i < chunkSize; i++) {
		for(int j = 0; j < chunkSize; j++) {
			char backgroundTileColumn = 0;
			char backgroundTileRow = 0;
			char backgroundTileId = 0;
			char backgroundTileCollider = 0;

			char foregroundTileColumn = 0;
			char foregroundTileRow = 0;
			char foregroundTileId = 0;
			char foregroundTileCollider = 0;

			foregroundTileColumn = 0;
			foregroundTileRow = 0;
			foregroundTileId = 0;
			foregroundTileCollider = false;

			// fill background with grass
			int grassColumn = rand() % 3;
			int grassRow = rand() % 2;
			backgroundTileColumn = grassColumn;
			backgroundTileRow = grassRow;
			backgroundTileId = 2;
			foregroundTileCollider = false;

			// place boundary rocks
			if((i == chunkSize - 1 || j == chunkSize - 1) && i != 0 && j != 0) {
				backgroundTileColumn = 5;
				backgroundTileRow = 0;
				backgroundTileCollider = true;
			}

			g_string_append_c(chunk->backgroundTiles.tilesetColumns, backgroundTileColumn);
			g_string_append_c(chunk->backgroundTiles.tilesetRows, backgroundTileRow);
			g_string_append_c(chunk->backgroundTiles.tilesetIds, backgroundTileId);
			g_string_append_c(chunk->backgroundTiles.tilesetColliders, backgroundTileCollider);

			g_string_append_c(chunk->foregroundTiles.tilesetColumns, foregroundTileColumn);
			g_string_append_c(chunk->foregroundTiles.tilesetRows, foregroundTileRow);
			g_string_append_c(chunk->foregroundTiles.tilesetIds, foregroundTileId);
			g_string_append_c(chunk->foregroundTiles.tilesetColliders, foregroundTileCollider);
		}
	}

	return chunk;
}

static ChunkData *createChunk(int x, int z, int chunkSize)
{
	ChunkData *chunk = createBlankChunk(
		chunkSize,
		(float) x + (float) chunkSize / 2.0f,
		(float) z + (float) chunkSize / 2.0f);

	int rockSeedModulo = 5 + (rand() % 100);
	int bushSeedModulo = 5 + (rand() % 25);
	int treeSeedModulo = 5 + (rand() % 50);

	for(int i = 0; i < chunkSize; i++) {
		bool isBottomBorder = z == -halfMapHeight && i == 0;
		bool isTopBorder = z == halfMapHeight - chunkSize && i == chunkSize - 1;
		for(int j = 0; j < chunkSize; j++) {
			bool isLeftBorder = x == -halfMapWidth && j == 0;
			bool isRightBorder = x == halfMapWidth - chunkSize && j == chunkSize - 1;

			// map borders
			if(isLeftBorder || isRightBorder || isBottomBorder || isTopBorder) {
				// rock
				g_string_append_c(chunk->backgroundTiles.tilesetColumns, (char) 5);
				g_string_append_c(chunk->backgroundTiles.tilesetRows, (char) 0);
				g_string_append_c(chunk->backgroundTiles.tilesetIds, (char) 2);
				g_string_append_c(chunk->backgroundTiles.tilesetColliders, (char) true);

				// nothing
				g_string_append_c(chunk->foregroundTiles.tilesetColumns, (char) 0);
				g_string_append_c(chunk->foregroundTiles.tilesetRows, (char) 0);
				g_string_append_c(chunk->foregroundTiles.tilesetIds, (char) 0);
				g_string_append_c(chunk->foregroundTiles.tilesetColliders, (char) false);

				continue;
			}

			char backgroundTileColumn = 0;
			char backgroundTileRow = 0;
			char backgroundTileId = 0;
			char backgroundTileCollider = 0;

			char foregroundTileColumn = 0;
			char foregroundTileRow = 0;
			char foregroundTileId = 0;
			char foregroundTileCollider = 0;

			foregroundTileColumn = 0;
			foregroundTileRow = 0;
			foregroundTileId = 0;
			foregroundTileCollider = false;

			// fill background with grass
			int grassColumn = rand() % 3;
			int grassRow = rand() % 2;
			backgroundTileColumn = grassColumn;
			backgroundTileRow = grassRow;
			backgroundTileId = 2;
			foregroundTileCollider = false;

			// place rocks
			int rockSeed = rand() % rockSeedModulo;
			if(rockSeed == 0) {
				backgroundTileColumn = 5;
				backgroundTileRow = 0;
				backgroundTileCollider = true;
			}

			// place bushes
			int bushSeed = rand() % bushSeedModulo;
			if(bushSeed == 0) {
				backgroundTileColumn = 5;
				backgroundTileRow = 1;
				backgroundTileCollider = true;
			}

			// place trees
			do {
				if(i > 0) {
					char previousRowTileColumn = chunk->backgroundTiles.tilesetColumns->str[(i - 1) * chunkSize + j];
					char previousRowTileRow = chunk->backgroundTiles.tilesetRows->str[(i - 1) * chunkSize + j];

					// complete left part of tree
					if(previousRowTileColumn == 3 && previousRowTileRow == 0) {
						foregroundTileColumn = 3;
						foregroundTileRow = 1;
						foregroundTileId = 2;
						break;
					}

					// complete right part of tree
					if(previousRowTileColumn == 4 && previousRowTileRow == 0) {
						foregroundTileColumn = 4;
						foregroundTileRow = 1;
						foregroundTileId = 2;
						break;
					}
				}

				if(j > 0) {
					char previousTileColumn = chunk->backgroundTiles.tilesetColumns
						->str[chunk->backgroundTiles.tilesetColumns->len - 1];
					char previousTileRow = chunk->backgroundTiles.tilesetRows
						->str[chunk->backgroundTiles.tilesetRows->len - 1];

					// complete bottom part of tree
					if (previousTileColumn == 3 && previousTileRow == 0) {
						backgroundTileColumn = 4;
						backgroundTileRow = 0;
						backgroundTileCollider = true;
						break;
					}
				}

				if(i > 0 && j < chunkSize - 1) {
					char previousRowNextTileColumn = chunk->backgroundTiles.tilesetColumns->str[(i - 1) * chunkSize + j + 1];
					char previousRowNextTileRow = chunk->backgroundTiles.tilesetRows->str[(i - 1) * chunkSize + j + 1];

					// reject positions that already have a tree starting on the bottom right neighbor tile
					if (previousRowNextTileColumn == 3 && previousRowNextTileRow == 0) {
						break;
					}
				}

				// reject positions that don't fit into the chunk
				if(i == chunkSize - 1 || j == chunkSize - 1) {
					break;
				}

				// start new tree
				int treeSeed = rand() % treeSeedModulo;
				if(treeSeed == 0) {
					backgroundTileColumn = 3;
					backgroundTileRow = 0;
					backgroundTileCollider = true;
				}
			} while (false);

			g_string_append_c(chunk->backgroundTiles.tilesetColumns, backgroundTileColumn);
			g_string_append_c(chunk->backgroundTiles.tilesetRows, backgroundTileRow);
			g_string_append_c(chunk->backgroundTiles.tilesetIds, backgroundTileId);
			g_string_append_c(chunk->backgroundTiles.tilesetColliders, backgroundTileCollider);

			g_string_append_c(chunk->foregroundTiles.tilesetColumns, foregroundTileColumn);
			g_string_append_c(chunk->foregroundTiles.tilesetRows, foregroundTileRow);
			g_string_append_c(chunk->foregroundTiles.tilesetIds, foregroundTileId);
			g_string_append_c(chunk->foregroundTiles.tilesetColliders, foregroundTileCollider);
		}
	}

	return chunk;
}
