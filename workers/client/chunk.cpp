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
		configuration.positionMappingX = convertCoordinateMapping(op.Data.position_mapping_x());
		configuration.positionMappingY = convertCoordinateMapping(op.Data.position_mapping_y());
		const Vector2& size = op.Data.size();
		configuration.size = shovelerVector2(size.x(), size.y());
		configuration.collider = op.Data.collider();
		const List<ChunkLayer>& layers = op.Data.layers();
		configuration.numLayers = layers.size();
		configuration.layers = new ShovelerViewChunkLayerConfiguration[configuration.numLayers];
		for(int i = 0; i < configuration.numLayers; ++i) {
			configuration.layers[i].type = convertLayerType(layers[i].type());
			configuration.layers[i].valueEntityId = layers[i].value_entity_id();
		}

		shovelerViewEntityAddChunk(entity, &configuration);

		delete[] configuration.layers;
	});

	dispatcher.OnComponentUpdate<Chunk>([&, view](const worker::ComponentUpdateOp<Chunk>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		const ShovelerViewChunkConfiguration *currentConfiguration = shovelerViewEntityGetChunkConfiguration(entity);

		ShovelerViewChunkConfiguration configuration;

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

		if(op.Update.collider()) {
			configuration.collider = *op.Update.collider();
		}

		if(op.Update.layers()) {
			const List<ChunkLayer>& layers = *op.Update.layers();
			configuration.numLayers = layers.size();
			configuration.layers = new ShovelerViewChunkLayerConfiguration[configuration.numLayers];
			for(int i = 0; i < configuration.numLayers; ++i) {
				configuration.layers[i].type = convertLayerType(layers[i].type());
				configuration.layers[i].valueEntityId = layers[i].value_entity_id();
			}
		} else {
			configuration.numLayers = currentConfiguration->numLayers;
			configuration.layers = new ShovelerViewChunkLayerConfiguration[configuration.numLayers];
			for(int i = 0; i < configuration.numLayers; ++i) {
				configuration.layers[i] = currentConfiguration->layers[i];
			}
		}

		shovelerViewEntityUpdateChunk(entity, configuration);

		delete[] configuration.layers;
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
