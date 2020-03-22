#include <cstdlib>

#include "map.h"

using worker::Entity;
using worker::List;

static const int halfMapWidth = 100;
static const int halfMapHeight = 100;

static ChunkData createStartingAreaBottomLeft(int chunkSize);
static ChunkData createStartingAreaBottomRight(int chunkSize);
static ChunkData createStartingAreaTopLeft(int chunkSize);
static ChunkData createStartingAreaTopRight(int chunkSize);
static ChunkData createChunk(int x, int y, int chunkSize);

List<ChunkData> generateMapChunks(int chunkSize)
{
	List<ChunkData> chunks;

	for(int x = -halfMapWidth; x < halfMapWidth; x += chunkSize) {
		for(int z = -halfMapHeight; z < halfMapHeight; z += chunkSize) {
			if(x == -chunkSize && z == -chunkSize) {
				chunks.emplace_back(createStartingAreaBottomLeft(chunkSize));
			} else if(x == 0 && z == -chunkSize) {
				chunks.emplace_back(createStartingAreaBottomRight(chunkSize));
			} else if(x == -chunkSize && z == 0) {
				chunks.emplace_back(createStartingAreaTopLeft(chunkSize));
			} else if(x == 0 && z == 0) {
				chunks.emplace_back(createStartingAreaTopRight(chunkSize));
			} else {
				chunks.emplace_back(createChunk(x, z, chunkSize));
			}
		}
	}

	return chunks;
}

static ChunkData createStartingAreaBottomLeft(int chunkSize)
{
	ChunkData chunk;
	chunk.position = shovelerVector2(-chunkSize / 2.0f, -chunkSize / 2.0f);

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

			chunk.backgroundTiles.tilesetColumns += backgroundTileColumn;
			chunk.backgroundTiles.tilesetRows += backgroundTileRow;
			chunk.backgroundTiles.tilesetIds += backgroundTileId;
			chunk.backgroundTiles.tilesetColliders += backgroundTileCollider;

			chunk.foregroundTiles.tilesetColumns += foregroundTileColumn;
			chunk.foregroundTiles.tilesetRows += foregroundTileRow;
			chunk.foregroundTiles.tilesetIds += foregroundTileId;
			chunk.foregroundTiles.tilesetColliders += foregroundTileCollider;
		}
	}

	return chunk;
}

static ChunkData createStartingAreaBottomRight(int chunkSize)
{
	ChunkData chunk;
	chunk.position = shovelerVector2(chunkSize / 2.0f, -chunkSize / 2.0f);

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

			chunk.backgroundTiles.tilesetColumns += backgroundTileColumn;
			chunk.backgroundTiles.tilesetRows += backgroundTileRow;
			chunk.backgroundTiles.tilesetIds += backgroundTileId;
			chunk.backgroundTiles.tilesetColliders += backgroundTileCollider;

			chunk.foregroundTiles.tilesetColumns += foregroundTileColumn;
			chunk.foregroundTiles.tilesetRows += foregroundTileRow;
			chunk.foregroundTiles.tilesetIds += foregroundTileId;
			chunk.foregroundTiles.tilesetColliders += foregroundTileCollider;
		}
	}

	return chunk;
}

static ChunkData createStartingAreaTopLeft(int chunkSize)
{
	ChunkData chunk;
	chunk.position = shovelerVector2(-chunkSize / 2.0f, chunkSize / 2.0f);

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

			chunk.backgroundTiles.tilesetColumns += backgroundTileColumn;
			chunk.backgroundTiles.tilesetRows += backgroundTileRow;
			chunk.backgroundTiles.tilesetIds += backgroundTileId;
			chunk.backgroundTiles.tilesetColliders += backgroundTileCollider;

			chunk.foregroundTiles.tilesetColumns += foregroundTileColumn;
			chunk.foregroundTiles.tilesetRows += foregroundTileRow;
			chunk.foregroundTiles.tilesetIds += foregroundTileId;
			chunk.foregroundTiles.tilesetColliders += foregroundTileCollider;
		}
	}

	return chunk;
}

static ChunkData createStartingAreaTopRight(int chunkSize)
{
	ChunkData chunk;
	chunk.position = shovelerVector2(chunkSize / 2.0f, chunkSize / 2.0f);

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

			chunk.backgroundTiles.tilesetColumns += backgroundTileColumn;
			chunk.backgroundTiles.tilesetRows += backgroundTileRow;
			chunk.backgroundTiles.tilesetIds += backgroundTileId;
			chunk.backgroundTiles.tilesetColliders += backgroundTileCollider;

			chunk.foregroundTiles.tilesetColumns += foregroundTileColumn;
			chunk.foregroundTiles.tilesetRows += foregroundTileRow;
			chunk.foregroundTiles.tilesetIds += foregroundTileId;
			chunk.foregroundTiles.tilesetColliders += foregroundTileCollider;
		}
	}

	return chunk;
}

static ChunkData createChunk(int x, int z, int chunkSize)
{
	ChunkData chunk;
	chunk.position = shovelerVector2(x + chunkSize / 2.0f, z + chunkSize / 2.0f);

	int rockSeedModulo = 5 + (rand() % 100);
	int bushSeedModulo = 5 + (rand() % 25);
	int treeSeedModulo = 5 + (rand() % 50);

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
					char previousRowTileColumn = chunk.backgroundTiles.tilesetColumns[(i - 1) * chunkSize + j];
					char previousRowTileRow = chunk.backgroundTiles.tilesetRows[(i - 1) * chunkSize + j];

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
					char previousTileColumn = *chunk.backgroundTiles.tilesetColumns.rbegin();
					char previousTileRow = *chunk.backgroundTiles.tilesetRows.rbegin();

					// complete bottom part of tree
					if (previousTileColumn == 3 && previousTileRow == 0) {
						backgroundTileColumn = 4;
						backgroundTileRow = 0;
						backgroundTileCollider = true;
						break;
					}
				}

				if(i > 0 && j < chunkSize - 1) {
					char previousRowNextTileColumn = chunk.backgroundTiles.tilesetColumns[(i - 1) * chunkSize + j + 1];
					char previousRowNextTileRow = chunk.backgroundTiles.tilesetRows[(i - 1) * chunkSize + j + 1];

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

			chunk.backgroundTiles.tilesetColumns += backgroundTileColumn;
			chunk.backgroundTiles.tilesetRows += backgroundTileRow;
			chunk.backgroundTiles.tilesetIds += backgroundTileId;
			chunk.backgroundTiles.tilesetColliders += backgroundTileCollider;

			chunk.foregroundTiles.tilesetColumns += foregroundTileColumn;
			chunk.foregroundTiles.tilesetRows += foregroundTileRow;
			chunk.foregroundTiles.tilesetIds += foregroundTileId;
			chunk.foregroundTiles.tilesetColliders += foregroundTileCollider;
		}
	}

	return chunk;
}
