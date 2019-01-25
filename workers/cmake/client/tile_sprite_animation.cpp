#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tile_sprite_animation.h"

extern "C" {
#include <shoveler/view/tile_sprite_animation.h>
#include <shoveler/log.h>
}

using shoveler::CoordinateMapping;
using shoveler::TileSpriteAnimation;
using worker::EntityId;

void registerTileSpriteAnimationCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<TileSpriteAnimation>([&, view](const worker::AddComponentOp<TileSpriteAnimation>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTileSpriteAnimationConfiguration configuration;
		configuration.tileSpriteEntityId = op.Data.tile_sprite_entity_id() != 0 ? op.Data.tile_sprite_entity_id() : op.EntityId;
		configuration.moveAmountThreshold = op.Data.move_amount_threshold();

		shovelerViewEntityAddTileSpriteAnimation(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<TileSpriteAnimation>([&, view](const worker::ComponentUpdateOp<TileSpriteAnimation>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerViewTileSpriteAnimationConfiguration configuration = *shovelerViewEntityGetTileSpriteAnimationConfiguration(entity);

		if(op.Update.tile_sprite_entity_id()) {
			configuration.tileSpriteEntityId = *op.Update.tile_sprite_entity_id() != 0 ? *op.Update.tile_sprite_entity_id() : op.EntityId;
		}

		if(op.Update.move_amount_threshold()) {
			configuration.moveAmountThreshold = *op.Update.move_amount_threshold();
		}

		shovelerViewEntityUpdateTileSpriteAnimation(entity, &configuration);

	});

	dispatcher.OnRemoveComponent<TileSpriteAnimation>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTileSpriteAnimation(entity);
	});
}
