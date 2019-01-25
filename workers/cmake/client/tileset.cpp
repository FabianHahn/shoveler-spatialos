#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tileset.h"

extern "C" {
#include <shoveler/view/tileset.h>
#include <shoveler/log.h>
}

using shoveler::Tileset;

void registerTilesetCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Tileset>([&, view](const worker::AddComponentOp<Tileset>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTilesetConfiguration configuration;
		configuration.imageResourceEntityId = op.Data.image_resource_entity_id();
		configuration.columns = op.Data.columns();
		configuration.rows = op.Data.rows();
		configuration.padding = op.Data.padding();

		shovelerViewEntityAddTileset(entity, configuration);
	});

	dispatcher.OnComponentUpdate<Tileset>([&, view](const worker::ComponentUpdateOp<Tileset>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerViewTilesetConfiguration configuration = *shovelerViewEntityGetTilesetConfiguration(entity);

		if(op.Update.image_resource_entity_id()) {
			configuration.imageResourceEntityId = *op.Update.image_resource_entity_id();
		}

		if(op.Update.columns()) {
			configuration.columns = *op.Update.columns();
		}

		if(op.Update.rows()) {
			configuration.rows = *op.Update.rows();
		}

		if(op.Update.padding()) {
			configuration.padding = *op.Update.padding();
		}

		shovelerViewEntityUpdateTileset(entity, configuration);
	});

	dispatcher.OnRemoveComponent<Tileset>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTileset(entity);
	});
}
