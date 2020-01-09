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
		configuration.tilesEntityId = op.Data.tiles();
        configuration.collidersEntityId = op.Data.colliders();

		configuration.numTilesets = op.Data.tilesets().size();
        long long int *tilesetEntityIds = new long long int[configuration.numTilesets];
		{
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Data.tilesets().begin(); iter != op.Data.tilesets().end(); ++iter, ++i) {
				tilesetEntityIds[i] = *iter;
			}
		}
		configuration.tilesetEntityIds = tilesetEntityIds;

		shovelerViewEntityAddTilemap(entity, &configuration);

		delete[] tilesetEntityIds;
	});

	dispatcher.OnComponentUpdate<Tilemap>([&, view](const worker::ComponentUpdateOp<Tilemap>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTilemapConfiguration configuration;
		shovelerViewEntityGetTilemapConfiguration(entity, &configuration);

		if(op.Update.tiles()) {
			configuration.tilesEntityId = *op.Update.tiles();
		}

        if(op.Update.colliders()) {
            configuration.collidersEntityId = *op.Update.colliders();
        }

        long long int *tilesetEntityIds = NULL;
		if(op.Update.tilesets()) {
			configuration.numTilesets = op.Update.tilesets()->size();
            tilesetEntityIds = new long long int[configuration.numTilesets];
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Update.tilesets()->begin(); iter != op.Update.tilesets()->end(); ++iter, ++i) {
				tilesetEntityIds[i] = *iter;
			}
			configuration.tilesetEntityIds = tilesetEntityIds;
		}

		shovelerViewEntityUpdateTilemap(entity, &configuration);

		delete[] tilesetEntityIds;
	});

	dispatcher.OnRemoveComponent<Tilemap>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTilemap(entity);
	});
}
