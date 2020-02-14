#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "coordinate_mapping.h"
#include "tile_sprite.h"

extern "C" {
#include <shoveler/view/tile_sprite.h>
#include <shoveler/log.h>
}

using shoveler::CoordinateMapping;
using shoveler::TileSprite;
using shoveler::Vector2;
using worker::EntityId;
using worker::List;

void registerTileSpriteCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<TileSprite>([&, view](const worker::AddComponentOp<TileSprite>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTileSpriteConfiguration configuration;
		configuration.positionEntityId = op.Data.position();
		configuration.materialEntityId = op.Data.material();
		configuration.tilesetEntityId = op.Data.tileset();
		configuration.tilesetColumn = op.Data.tileset_column();
		configuration.tilesetRow = op.Data.tileset_row();
		configuration.positionMappingX = convertCoordinateMapping(op.Data.position_mapping_x());
		configuration.positionMappingY = convertCoordinateMapping(op.Data.position_mapping_y());

		const Vector2& size = op.Data.size();
		configuration.size = shovelerVector2(size.x(), size.y());

		shovelerViewEntityAddTileSprite(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<TileSprite>([&, view](const worker::ComponentUpdateOp<TileSprite>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTileSpriteConfiguration configuration;
		shovelerViewEntityGetTileSpriteConfiguration(entity, &configuration);

		if(op.Update.position()) {
			configuration.positionEntityId = *op.Update.position();
		}

		if(op.Update.material()) {
			configuration.materialEntityId = *op.Update.material();
		}

        if(op.Update.tileset()) {
            configuration.tilesetEntityId = *op.Update.tileset();
        }

		if(op.Update.tileset_column()) {
			configuration.tilesetColumn = *op.Update.tileset_column();
		}

		if(op.Update.tileset_row()) {
			configuration.tilesetRow = *op.Update.tileset_row();
		}

		if(op.Update.position_mapping_x()) {
			configuration.positionMappingX = convertCoordinateMapping(*op.Update.position_mapping_x());
		}

		if(op.Update.position_mapping_y()) {
			configuration.positionMappingY = convertCoordinateMapping(*op.Update.position_mapping_y());
		}

		if(op.Update.size()) {
			const Vector2& size = *op.Update.size();
			configuration.size = shovelerVector2(size.x(), size.y());
		}

		shovelerViewEntityUpdateTileSprite(entity, &configuration);

	});

	dispatcher.OnRemoveComponent<TileSprite>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTileSprite(entity);
	});
}
