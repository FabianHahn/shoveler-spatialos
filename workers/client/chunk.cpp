#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "chunk.h"
#include "coordinate_mapping.h"

extern "C" {
#include <shoveler/view/chunk.h>
#include <shoveler/log.h>
}

using shoveler::Chunk;
using shoveler::ChunkLayer;
using shoveler::ChunkLayerType;
using shoveler::CoordinateMapping;
using shoveler::Vector2;
using worker::EntityId;
using worker::List;

static ShovelerChunkLayerType convertLayerType(ChunkLayerType type);

void registerChunkCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Chunk>([&, view](const worker::AddComponentOp<Chunk>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewChunkConfiguration configuration;
		configuration.positionEntityId = op.Data.position();
		configuration.positionMappingX = convertCoordinateMapping(op.Data.position_mapping_x());
		configuration.positionMappingY = convertCoordinateMapping(op.Data.position_mapping_y());
		const Vector2& size = op.Data.size();
		configuration.size = shovelerVector2(size.x(), size.y());
		const List<EntityId>& layers = op.Data.layers();
		configuration.numLayers = layers.size();
        long long int *layerEntityIds = new long long int[configuration.numLayers];
		for(int i = 0; i < configuration.numLayers; ++i) {
            layerEntityIds[i] = layers[i];
		}
		configuration.layerEntityIds = layerEntityIds;

		shovelerViewEntityAddChunk(entity, &configuration);

		delete[] layerEntityIds;
	});

	dispatcher.OnComponentUpdate<Chunk>([&, view](const worker::ComponentUpdateOp<Chunk>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

        ShovelerViewChunkConfiguration configuration;
		shovelerViewEntityGetChunkConfiguration(entity, &configuration);

        if(op.Update.position()) {
            configuration.positionEntityId = *op.Update.position();
        }

		if(op.Update.position_mapping_x()) {
			configuration.positionMappingX = convertCoordinateMapping(*op.Update.position_mapping_x());
		}

		if(op.Update.position_mapping_y()) {
			configuration.positionMappingY = convertCoordinateMapping(*op.Update.position_mapping_y());
		}

		if(op.Update.size()) {
			const Vector2& size = *op.Update.size();
			configuration.size = shovelerVector2(size.x(), size.y());
		}

        long long int *layerEntityIds = NULL;
		if(op.Update.layers()) {
			const List<EntityId>& layers = *op.Update.layers();
			configuration.numLayers = layers.size();
            layerEntityIds = new long long int[configuration.numLayers];
			for(int i = 0; i < configuration.numLayers; ++i) {
                layerEntityIds[i] = layers[i];
			}
            configuration.layerEntityIds = layerEntityIds;
		}

		shovelerViewEntityUpdateChunk(entity, &configuration);

		if(layerEntityIds != NULL) {
            delete[] layerEntityIds;
		}
	});

	dispatcher.OnRemoveComponent<Chunk>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveChunk(entity);
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
