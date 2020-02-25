#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tileset.h"

extern "C" {
#include <shoveler/view/tilemap_tiles.h>
#include <shoveler/component.h>
#include <shoveler/log.h>
}

using shoveler::TilemapTiles;
using worker::EntityId;
using worker::List;

void registerTilemapTilesCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<TilemapTiles>([&, view](const worker::AddComponentOp<TilemapTiles>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		bool isImageResourceDefinition = bool(op.Data.image());
		bool isConfigurationOptionDefinition =
			bool(op.Data.num_columns()) &&
			bool(op.Data.num_rows()) &&
			bool(op.Data.tileset_columns()) &&
			bool(op.Data.tileset_rows()) &&
			bool(op.Data.tileset_ids());

		if(!(isImageResourceDefinition ^ isConfigurationOptionDefinition)) {
			shovelerLogWarning("Received add tilemap tiles component for entity %lld that is neither an image resource definition nor a configuration option definition.", op.EntityId);
			return;
		}

		ShovelerViewTilemapTilesConfiguration configuration;
		configuration.isImageResourceEntityDefinition = isImageResourceDefinition;

		unsigned char *tilesetColumns = NULL;
		unsigned char *tilesetRows = NULL;
		unsigned char *tilesetIds = NULL;

		if(configuration.isImageResourceEntityDefinition) {
			configuration.imageEntityId = *op.Data.image();
		} else {
			configuration.numColumns = *op.Data.num_columns();
			configuration.numRows = *op.Data.num_rows();

			tilesetColumns = new unsigned char[configuration.numColumns * configuration.numRows];
			tilesetRows = new unsigned char[configuration.numColumns * configuration.numRows];
			tilesetIds = new unsigned char[configuration.numColumns * configuration.numRows];

			const std::string& tilesetColumnsString = *op.Data.tileset_columns();
			const std::string& tilesetRowsString = *op.Data.tileset_rows();
			const std::string& tilesetIdsString = *op.Data.tileset_ids();

			for(int row = 0; row < configuration.numRows; ++row) {
				for(int column = 0; column < configuration.numColumns; ++column) {
					int tileIndex = row * configuration.numColumns + column;
					tilesetColumns[tileIndex] = tilesetColumnsString[tileIndex];
					tilesetRows[tileIndex] = tilesetRowsString[tileIndex];
					tilesetIds[tileIndex] = tilesetIdsString[tileIndex];
				}
			}
			configuration.tilesetColumns = tilesetColumns;
			configuration.tilesetRows = tilesetRows;
			configuration.tilesetIds = tilesetIds;
		}

		shovelerViewEntityAddTilemapTiles(entity, &configuration);

		if(!configuration.isImageResourceEntityDefinition) {
			delete[] tilesetColumns;
			delete[] tilesetRows;
			delete[] tilesetIds;
		}
	});

	dispatcher.OnComponentUpdate<TilemapTiles>([&, view](const worker::ComponentUpdateOp<TilemapTiles>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerComponent *component = shovelerViewEntityGetTilemapTilesComponent(entity);

		ShovelerViewTilemapTilesConfiguration configuration;
		shovelerViewEntityGetTilemapTilesConfiguration(entity, &configuration);

		if(op.Update.image()) {
			if(*op.Update.image()) {
				shovelerComponentUpdateCanonicalConfigurationOptionEntityId(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_IMAGE, **op.Update.image());
			} else {
				shovelerComponentClearConfigurationOption(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_IMAGE, /* isCanonical */ true);
			}
		}

		if(op.Update.num_columns()) {
			if(*op.Update.num_columns()) {
				shovelerComponentUpdateCanonicalConfigurationOptionInt(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_NUM_COLUMNS, **op.Update.num_columns());
			} else {
				shovelerComponentClearConfigurationOption(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_NUM_COLUMNS, /* isCanonical */ true);
			}
		}

		if(op.Update.num_rows()) {
			if(*op.Update.num_rows()) {
				shovelerComponentUpdateCanonicalConfigurationOptionInt(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_NUM_ROWS, **op.Update.num_rows());
			} else {
				shovelerComponentClearConfigurationOption(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_NUM_ROWS, /* isCanonical */ true);
			}
		}

		if(op.Update.tileset_columns()) {
			if(*op.Update.tileset_columns()) {
				unsigned char *tilesetColumns = new unsigned char[configuration.numColumns * configuration.numRows];

				const std::string& tilesetColumnsString = **op.Update.tileset_columns();

				for(int row = 0; row < configuration.numRows; ++row) {
					for(int column = 0; column < configuration.numColumns; ++column) {
						int tileIndex = row * configuration.numColumns + column;
						tilesetColumns[tileIndex] = tilesetColumnsString[tileIndex];
					}
				}

				shovelerComponentUpdateCanonicalConfigurationOptionBytes(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_TILESET_COLUMNS, tilesetColumns, configuration.numColumns * configuration.numRows);

				delete[] tilesetColumns;
			} else {
				shovelerComponentClearConfigurationOption(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_TILESET_COLUMNS, /* isCanonical */ true);
			}
		}

		if(op.Update.tileset_rows()) {
			if(*op.Update.tileset_rows()) {
				unsigned char *tilesetRows = new unsigned char[configuration.numColumns * configuration.numRows];

				const std::string& tilesetRowsString = **op.Update.tileset_rows();

				for(int row = 0; row < configuration.numRows; ++row) {
					for(int column = 0; column < configuration.numColumns; ++column) {
						int tileIndex = row * configuration.numColumns + column;
						tilesetRows[tileIndex] = tilesetRowsString[tileIndex];
					}
				}

				shovelerComponentUpdateCanonicalConfigurationOptionBytes(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_TILESET_ROWS, tilesetRows, configuration.numColumns * configuration.numRows);

				delete[] tilesetRows;
			} else {
				shovelerComponentClearConfigurationOption(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_TILESET_ROWS, /* isCanonical */ true);
			}
		}

		if(op.Update.tileset_ids()) {
			if(*op.Update.tileset_ids()) {
				unsigned char *tilesetIds = new unsigned char[configuration.numColumns * configuration.numRows];

				const std::string& tilesetIdsString = **op.Update.tileset_ids();

				for(int row = 0; row < configuration.numRows; ++row) {
					for(int column = 0; column < configuration.numColumns; ++column) {
						int tileIndex = row * configuration.numColumns + column;
						tilesetIds[tileIndex] = tilesetIdsString[tileIndex];
					}
				}

				shovelerComponentUpdateCanonicalConfigurationOptionBytes(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_TILESET_IDS, tilesetIds, configuration.numColumns * configuration.numRows);

				delete[] tilesetIds;
			} else {
				shovelerComponentClearConfigurationOption(component, SHOVELER_COMPONENT_TILEMAP_TILES_OPTION_TILESET_IDS, /* isCanonical */ true);
			}
		}

		shovelerComponentActivate(component);
	});

	dispatcher.OnRemoveComponent<TilemapTiles>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTilemapTiles(entity);
	});
}
