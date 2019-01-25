#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tileset.h"

extern "C" {
#include <shoveler/view/tilemap_tiles.h>
#include <shoveler/log.h>
}

using shoveler::TilemapTiles;
using shoveler::TilemapTilesTile;
using worker::EntityId;
using worker::List;

void registerTilemapTilesCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<TilemapTiles>([&, view](const worker::AddComponentOp<TilemapTiles>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTilemapTilesConfiguration configuration;
		configuration.isImageResourceEntityDefinition = op.Data.is_image_resource_entity_definition();
		configuration.imageResourceEntityId = op.Data.image_resource_entity_id();
		configuration.numColumns = op.Data.num_columns();
		configuration.numRows = op.Data.num_rows();

		const List<TilemapTilesTile>& tiles = op.Data.tiles();
		configuration.tiles = new ShovelerViewTilemapTilesTileConfiguration[configuration.numColumns * configuration.numRows];
		for(int row = 0; row < configuration.numRows; ++row) {
			for(int column = 0; column < configuration.numColumns; ++column) {
				int tileIndex = row * configuration.numColumns + column;
				configuration.tiles[tileIndex].tilesetColumn = tiles[tileIndex].tileset_column();
				configuration.tiles[tileIndex].tilesetRow = tiles[tileIndex].tileset_row();
				configuration.tiles[tileIndex].tilesetId = tiles[tileIndex].tileset_id();
			}
		}

		shovelerViewEntityAddTilemapTiles(entity, &configuration);

		delete[] configuration.tiles;
	});

	dispatcher.OnComponentUpdate<TilemapTiles>([&, view](const worker::ComponentUpdateOp<TilemapTiles>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		const ShovelerViewTilemapTilesConfiguration *currentConfiguration = shovelerViewEntityGetTilemapTilesConfiguration(entity);

		bool updateFullConfiguration = false;
		ShovelerViewTilemapTilesConfiguration configuration;

		if(op.Update.is_image_resource_entity_definition()) {
			configuration.isImageResourceEntityDefinition = *op.Update.is_image_resource_entity_definition();
			updateFullConfiguration = true;
		} else {
			configuration.isImageResourceEntityDefinition = currentConfiguration->isImageResourceEntityDefinition;
		}

		if(op.Update.image_resource_entity_id()) {
			configuration.imageResourceEntityId = *op.Update.image_resource_entity_id();
			updateFullConfiguration = true;
		} else {
			configuration.imageResourceEntityId = currentConfiguration->imageResourceEntityId;
		}

		if(op.Update.num_columns()) {
			configuration.numColumns = *op.Update.num_columns();
			updateFullConfiguration = true;
		} else {
			configuration.numColumns = currentConfiguration->numColumns;
		}

		if(op.Update.num_rows()) {
			configuration.numRows = *op.Update.num_rows();
			updateFullConfiguration = true;
		} else {
			configuration.numRows = currentConfiguration->numRows;
		}

		bool freeTiles = false;
		configuration.tiles = currentConfiguration->tiles;
		if(op.Update.tiles()) {
			const List<TilemapTilesTile>& tiles = *op.Update.tiles();
			if(updateFullConfiguration) {
				configuration.tiles = new ShovelerViewTilemapTilesTileConfiguration[configuration.numColumns * configuration.numRows];
				freeTiles = true;
			}

			for(int row = 0; row < configuration.numRows; ++row) {
				for(int column = 0; column < configuration.numColumns; ++column) {
					int tileIndex = row * configuration.numColumns + column;
					configuration.tiles[tileIndex].tilesetColumn = tiles[tileIndex].tileset_column();
					configuration.tiles[tileIndex].tilesetRow = tiles[tileIndex].tileset_row();
					configuration.tiles[tileIndex].tilesetId = tiles[tileIndex].tileset_id();
				}
			}
		}

		if(updateFullConfiguration) {
			shovelerViewEntityUpdateTilemapTiles(entity, &configuration);
		} else {
			shovelerViewEntityUpdateTilemapTilesData(entity, configuration.tiles);
		}

		if(freeTiles) {
			delete[] configuration.tiles;
		}
	});

	dispatcher.OnRemoveComponent<TilemapTiles>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTilemapTiles(entity);
	});
}
