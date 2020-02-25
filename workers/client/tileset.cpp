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
		configuration.imageEntityId = op.Data.image();
		configuration.numColumns = op.Data.num_columns();
		configuration.numRows = op.Data.num_rows();
		configuration.padding = op.Data.padding();

		shovelerViewEntityAddTileset(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Tileset>([&, view](const worker::ComponentUpdateOp<Tileset>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTilesetConfiguration configuration;
		shovelerViewEntityGetTilesetConfiguration(entity, &configuration);

		if(op.Update.image()) {
			configuration.imageEntityId = *op.Update.image();
		}

		if(op.Update.num_columns()) {
			configuration.numColumns = *op.Update.num_columns();
		}

		if(op.Update.num_rows()) {
			configuration.numRows = *op.Update.num_rows();
		}

		if(op.Update.padding()) {
			configuration.padding = *op.Update.padding();
		}

		shovelerViewEntityUpdateTileset(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<Tileset>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTileset(entity);
	});
}
