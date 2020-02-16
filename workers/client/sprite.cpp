#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "coordinate_mapping.h"
#include "tile_sprite.h"

extern "C" {
#include <shoveler/view/sprite.h>
#include <shoveler/log.h>
}

using shoveler::CoordinateMapping;
using shoveler::Sprite;
using shoveler::Vector2;
using worker::EntityId;
using worker::List;

void registerSpriteCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Sprite>([&, view](const worker::AddComponentOp<Sprite>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewSpriteConfiguration configuration;
		configuration.positionEntityId = op.Data.position();
		configuration.positionMappingX = convertCoordinateMapping(op.Data.position_mapping_x());
		configuration.positionMappingY = convertCoordinateMapping(op.Data.position_mapping_y());
		configuration.canvasEntityId = op.Data.canvas();
		configuration.layer = op.Data.layer();
		const Vector2& size = op.Data.size();
		configuration.size = shovelerVector2(size.x(), size.y());
		configuration.tileSpriteEntityId = op.Data.tile_sprite();

		shovelerViewEntityAddSprite(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Sprite>([&, view](const worker::ComponentUpdateOp<Sprite>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewSpriteConfiguration configuration;
		shovelerViewEntityGetSpriteConfiguration(entity, &configuration);

		if(op.Update.position()) {
			configuration.positionEntityId = *op.Update.position();
		}

		if(op.Update.position_mapping_x()) {
			configuration.positionMappingX = convertCoordinateMapping(*op.Update.position_mapping_x());
		}

		if(op.Update.position_mapping_y()) {
			configuration.positionMappingY = convertCoordinateMapping(*op.Update.position_mapping_y());
		}

		if(op.Update.canvas()) {
			configuration.canvasEntityId = *op.Update.canvas();
		}

		if(op.Update.layer()) {
			configuration.layer = *op.Update.layer();
		}

		if(op.Update.size()) {
			const Vector2& size = *op.Update.size();
			configuration.size = shovelerVector2(size.x(), size.y());
		}

		if(op.Update.tile_sprite()) {
			configuration.tileSpriteEntityId = *op.Update.tile_sprite();
		}

		shovelerViewEntityUpdateSprite(entity, &configuration);

	});

	dispatcher.OnRemoveComponent<Sprite>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveSprite(entity);
	});
}
