#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tileset.h"

extern "C" {
#include <shoveler/view/tilemap.h>
#include <shoveler/log.h>
}

using shoveler::Tilemap;
using worker::EntityId;
using worker::List;

void registerTilemapCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Tilemap>([&, view](const worker::AddComponentOp<Tilemap>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTilemapConfiguration configuration;
		configuration.tilesEntityId = op.Data.tiles_entity_id();

		configuration.numTilesets = op.Data.tileset_entity_ids().size();
		configuration.tilesetEntityIds = new long long int[configuration.numTilesets];
		{
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Data.tileset_entity_ids().begin(); iter != op.Data.tileset_entity_ids().end(); ++iter, ++i) {
				configuration.tilesetEntityIds[i] = *iter;
			}
		}

		shovelerViewEntityAddTilemap(entity, &configuration);

		delete[] configuration.tilesetEntityIds;
	});

	dispatcher.OnComponentUpdate<Tilemap>([&, view](const worker::ComponentUpdateOp<Tilemap>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		const ShovelerViewTilemapConfiguration *currentConfiguration = shovelerViewEntityGetTilemapConfiguration(entity);

		ShovelerViewTilemapConfiguration configuration;
		if(op.Update.tiles_entity_id()) {
			configuration.tilesEntityId = *op.Update.tiles_entity_id();
		} else {
			configuration.tilesEntityId = currentConfiguration->tilesEntityId;
		}

		if(op.Update.tileset_entity_ids()) {
			configuration.numTilesets = op.Update.tileset_entity_ids()->size();
			configuration.tilesetEntityIds = new long long int[configuration.numTilesets];
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Update.tileset_entity_ids()->begin(); iter != op.Update.tileset_entity_ids()->end(); ++iter, ++i) {
				configuration.tilesetEntityIds[i] = *iter;
			}
		} else {
			configuration.numTilesets = currentConfiguration->numTilesets;
			configuration.tilesetEntityIds = new long long int[configuration.numTilesets];
			for(int i = 0; i < configuration.numTilesets; ++i) {
				configuration.tilesetEntityIds[i] = currentConfiguration->tilesetEntityIds[i];
			}
		}

		shovelerViewEntityUpdateTilemap(entity, configuration);

		delete[] configuration.tilesetEntityIds;
	});

	dispatcher.OnRemoveComponent<Tilemap>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTilemap(entity);
	});
}
