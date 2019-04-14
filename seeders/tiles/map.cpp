#include <cstdlib>

#include "map.h"

using worker::Entity;
using worker::List;
using shoveler::TilemapTilesTile;

static const int halfMapWidth = 100;
static const int halfMapHeight = 100;

static ChunkData createChunk(int x, int y, int chunkSize);

List<ChunkData> generateMapChunks(int chunkSize)
{
	List<ChunkData> chunks;

	for(int x = -halfMapWidth; x < halfMapWidth; x += chunkSize) {
		for(int z = -halfMapHeight; z < halfMapHeight; z += chunkSize) {
			chunks.emplace_back(createChunk(x, z, chunkSize));
		}
	}

	return chunks;
}

static ChunkData createChunk(int x, int z, int chunkSize)
{
	ChunkData chunk;
	chunk.position = {x + chunkSize / 2.0, 0.0, z + chunkSize / 2.0};

	int rockSeedModulo = 5 + (rand() % 100);
	int bushSeedModulo = 5 + (rand() % 25);
	int treeSeedModulo = 5 + (rand() % 50);

	for(int i = 0; i < chunkSize; i++) {
		for(int j = 0; j < chunkSize; j++) {
			TilemapTilesTile backgroundTile;
			TilemapTilesTile foregroundTile;

			foregroundTile.set_tileset_column(0);
			foregroundTile.set_tileset_row(0);
			foregroundTile.set_tileset_id(0);
			foregroundTile.set_colliding(false);

			// fill background with grass
			int grassColumn = rand() % 3;
			int grassRow = rand() % 2;
			backgroundTile.set_tileset_column(grassColumn);
			backgroundTile.set_tileset_row(grassRow);
			backgroundTile.set_tileset_id(2);
			foregroundTile.set_colliding(false);

			// place rocks
			int rockSeed = rand() % rockSeedModulo;
			if(rockSeed == 0) {
				backgroundTile.set_tileset_column(5);
				backgroundTile.set_tileset_row(0);
				backgroundTile.set_colliding(true);
			}

			// place bushes
			int bushSeed = rand() % bushSeedModulo;
			if(bushSeed == 0) {
				backgroundTile.set_tileset_column(5);
				backgroundTile.set_tileset_row(1);
				backgroundTile.set_colliding(true);
			}

			// place trees
			do {
				if(i > 0) {
					const TilemapTilesTile& previousRow = chunk.backgroundTiles[(i - 1) * chunkSize + j];

					// complete left part of tree
					if(previousRow.tileset_column() == 3 && previousRow.tileset_row() == 0) {
						foregroundTile.set_tileset_column(3);
						foregroundTile.set_tileset_row(1);
						foregroundTile.set_tileset_id(2);
						break;
					}

					// complete right part of tree
					if(previousRow.tileset_column() == 4 && previousRow.tileset_row() == 0) {
						foregroundTile.set_tileset_column(4);
						foregroundTile.set_tileset_row(1);
						foregroundTile.set_tileset_id(2);
						break;
					}
				}

				if(j > 0) {
					const TilemapTilesTile &previousTile = *chunk.backgroundTiles.rbegin();

					// complete bottom part of tree
					if (previousTile.tileset_column() == 3 && previousTile.tileset_row() == 0) {
						backgroundTile.set_tileset_column(4);
						backgroundTile.set_tileset_row(0);
						backgroundTile.set_colliding(true);
						break;
					}
				}

				if(i > 0 && j < chunkSize - 1) {
					const TilemapTilesTile& previousRowNext = chunk.backgroundTiles[(i - 1) * chunkSize + j + 1];
					// reject positions that already have a tree starting on the bottom right neighbor tile
					if (previousRowNext.tileset_column() == 3 && previousRowNext.tileset_row() == 0) {
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
					backgroundTile.set_tileset_column(3);
					backgroundTile.set_tileset_row(0);
					backgroundTile.set_colliding(true);
				}
			} while (false);

			chunk.backgroundTiles.emplace_back(backgroundTile);
			chunk.foregroundTiles.emplace_back(foregroundTile);
		}
	}

	return chunk;
}
