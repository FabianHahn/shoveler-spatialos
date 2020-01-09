#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tileset.h"

extern "C" {
#include <shoveler/view/canvas.h>
#include <shoveler/component.h>
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
		configuration.numTileSprites = op.Data.tile_sprites().size();
        long long int *tileSpriteEntityIds = new long long int[configuration.numTileSprites];
		{
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Data.tile_sprites().begin(); iter != op.Data.tile_sprites().end(); ++iter, ++i) {
				tileSpriteEntityIds[i] = *iter;
			}
		}
		configuration.tileSpriteEntityIds = tileSpriteEntityIds;

		shovelerViewEntityAddCanvas(entity, &configuration);

		delete[] tileSpriteEntityIds;
	});

	dispatcher.OnComponentUpdate<Canvas>([&, view](const worker::ComponentUpdateOp<Canvas>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerComponent *component = shovelerViewEntityGetCanvasComponent(entity);

        ShovelerViewCanvasConfiguration configuration;
		shovelerViewEntityGetCanvasConfiguration(entity, &configuration);

        long long int *tileSpriteEntityIds = NULL;
		if(op.Update.tile_sprites()) {
			configuration.numTileSprites = op.Update.tile_sprites()->size();
            tileSpriteEntityIds = new long long int[configuration.numTileSprites];
			int i = 0;
			for(List<EntityId>::const_iterator iter = op.Update.tile_sprites()->begin(); iter != op.Update.tile_sprites()->end(); ++iter, ++i) {
				tileSpriteEntityIds[i] = *iter;
			}
			configuration.tileSpriteEntityIds = tileSpriteEntityIds;
		}

		shovelerViewEntityUpdateCanvas(entity, &configuration);

		if(tileSpriteEntityIds != NULL) {
            delete[] tileSpriteEntityIds;
        }

		shovelerComponentActivate(component);
	});

	dispatcher.OnRemoveComponent<Canvas>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveCanvas(entity);
	});
}
