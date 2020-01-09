#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "chunk.h"
#include "coordinate_mapping.h"

extern "C" {
#include <shoveler/view/chunk_layer.h>
#include <shoveler/log.h>
}

using shoveler::ChunkLayer;
using shoveler::ChunkLayerType;
using shoveler::CoordinateMapping;
using shoveler::Vector2;
using worker::EntityId;
using worker::List;

static ShovelerChunkLayerType convertLayerType(ChunkLayerType type);

void registerChunkLayerCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<ChunkLayer>([&, view](const worker::AddComponentOp<ChunkLayer>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewChunkLayerConfiguration configuration;
		configuration.type = convertLayerType(op.Data.type());

		if (configuration.type == SHOVELER_CHUNK_LAYER_TYPE_CANVAS) {
            configuration.canvasEntityId = *op.Data.canvas();
		} else {
            configuration.tilemapEntityId = *op.Data.tilemap();
		}

		shovelerViewEntityAddChunkLayer(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<ChunkLayer>([&, view](const worker::ComponentUpdateOp<ChunkLayer>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

        ShovelerViewChunkLayerConfiguration configuration;
		shovelerViewEntityGetChunkLayerConfiguration(entity, &configuration);

		if(op.Update.type()) {
			configuration.type = convertLayerType(*op.Update.type());
		}

		if(op.Update.canvas()) {
			configuration.canvasEntityId = **op.Update.canvas();
		}

        if(op.Update.tilemap()) {
            configuration.tilemapEntityId = **op.Update.tilemap();
        }

		shovelerViewEntityUpdateChunkLayer(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<ChunkLayer>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveChunkLayer(entity);
	});
}

static ShovelerChunkLayerType convertLayerType(ChunkLayerType type)
{
	switch(type) {
		case ChunkLayerType::CANVAS:
			return SHOVELER_CHUNK_LAYER_TYPE_CANVAS;
		case ChunkLayerType::TILEMAP:
			return SHOVELER_CHUNK_LAYER_TYPE_TILEMAP;
	}
}
