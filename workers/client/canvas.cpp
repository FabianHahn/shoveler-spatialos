#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tileset.h"

extern "C" {
#include <shoveler/view/canvas.h>
#include <shoveler/log.h>
}

using shoveler::Canvas;
using worker::EntityId;
using worker::List;

void registerCanvasCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Canvas>([&, view](const worker::AddComponentOp<Canvas>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewCanvasConfiguration configuration;
		configuration.numTileSprites = op.Data.tile_sprite_entity_ids().size();
		configuration.tileSpriteEntityIds = new long long int[configuration.numTileSprites];
		{
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Data.tile_sprite_entity_ids().begin(); iter != op.Data.tile_sprite_entity_ids().end(); ++iter, ++i) {
				configuration.tileSpriteEntityIds[i] = *iter;
			}
		}

		shovelerViewEntityAddCanvas(entity, &configuration);

		delete[] configuration.tileSpriteEntityIds;
	});

	dispatcher.OnComponentUpdate<Canvas>([&, view](const worker::ComponentUpdateOp<Canvas>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		const ShovelerViewCanvasConfiguration *currentConfiguration = shovelerViewEntityGetCanvasConfiguration(entity);

		ShovelerViewCanvasConfiguration configuration;
		if(op.Update.tile_sprite_entity_ids()) {
			configuration.numTileSprites = op.Update.tile_sprite_entity_ids()->size();
			configuration.tileSpriteEntityIds = new long long int[configuration.numTileSprites];
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Update.tile_sprite_entity_ids()->begin(); iter != op.Update.tile_sprite_entity_ids()->end(); ++iter, ++i) {
				configuration.tileSpriteEntityIds[i] = *iter;
			}
		} else {
			configuration.numTileSprites = currentConfiguration->numTileSprites;
			configuration.tileSpriteEntityIds = new long long int[configuration.numTileSprites];
			for(int i = 0; i < configuration.numTileSprites; ++i) {
				configuration.tileSpriteEntityIds[i] = currentConfiguration->tileSpriteEntityIds[i];
			}
		}

		shovelerViewEntityUpdateCanvas(entity, configuration);

		delete[] configuration.tileSpriteEntityIds;
	});

	dispatcher.OnRemoveComponent<Canvas>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveCanvas(entity);
	});
}
