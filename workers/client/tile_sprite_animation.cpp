#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tile_sprite_animation.h"
#include "coordinate_mapping.h"

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
		configuration.positionEntityId = op.Data.position();
		configuration.tileSpriteEntityId = op.Data.tile_sprite();
		configuration.positionMappingX = convertCoordinateMapping(op.Data.position_mapping_x());
        configuration.positionMappingY = convertCoordinateMapping(op.Data.position_mapping_y());
		configuration.moveAmountThreshold = op.Data.move_amount_threshold();

		shovelerViewEntityAddTileSpriteAnimation(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<TileSpriteAnimation>([&, view](const worker::ComponentUpdateOp<TileSpriteAnimation>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTileSpriteAnimationConfiguration configuration;
		shovelerViewEntityGetTileSpriteAnimationConfiguration(entity, &configuration);

        if(op.Update.position()) {
            configuration.positionEntityId = *op.Update.position();
        }

		if(op.Update.tile_sprite()) {
			configuration.tileSpriteEntityId = *op.Update.tile_sprite();
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
