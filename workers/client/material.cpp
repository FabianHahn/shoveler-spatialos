#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "material.h"

extern "C" {
#include <shoveler/view/material.h>
#include <shoveler/log.h>
}

using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Vector2;
using shoveler::Vector3;

static ShovelerComponentMaterialType convertMaterialType(MaterialType type);

void registerMaterialCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Material>([&, view](const worker::AddComponentOp<Material>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewMaterialConfiguration configuration;
		configuration.type = convertMaterialType(op.Data.type());

		switch(op.Data.type()) {
            case MaterialType::COLOR: {
                configuration.type = SHOVELER_COMPONENT_MATERIAL_TYPE_COLOR;
                Vector3 color = op.Data.color().value_or(Vector3{0, 0, 0});
                configuration.color = shovelerVector3(color.x(), color.y(), color.z());
            } break;
            case MaterialType::TEXTURE:
                configuration.type = SHOVELER_COMPONENT_MATERIAL_TYPE_TEXTURE;
                configuration.textureEntityId = *op.Data.texture();
                configuration.textureSamplerEntityId = *op.Data.texture_sampler();
                break;
            case MaterialType::PARTICLE: {
                configuration.type = SHOVELER_COMPONENT_MATERIAL_TYPE_PARTICLE;
                Vector3 color = op.Data.color().value_or(Vector3{0, 0, 0});
                configuration.color = shovelerVector3(color.x(), color.y(), color.z());
            } break;
            case MaterialType::TILEMAP:
                configuration.type = SHOVELER_COMPONENT_MATERIAL_TYPE_TILEMAP;
                configuration.tilemapEntityId = *op.Data.tilemap();
                break;
            case MaterialType::CANVAS: {
                configuration.type = SHOVELER_COMPONENT_MATERIAL_TYPE_CANVAS;
                configuration.canvasEntityId = *op.Data.canvas();

                Vector2 position = op.Data.canvas_region_position().value_or(Vector2{0.0f, 0.0f});
                configuration.canvasRegionPosition = shovelerVector2(position.x(), position.y());
                Vector2 size = op.Data.canvas_region_size().value_or(Vector2{0.0f, 0.0f});
                configuration.canvasRegionSize = shovelerVector2(size.x(), size.y());
            } break;
            case MaterialType::CHUNK:
                configuration.type = SHOVELER_COMPONENT_MATERIAL_TYPE_CHUNK;
                configuration.chunkEntityId = *op.Data.chunk();
                break;
		}

		shovelerViewEntityAddMaterial(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Material>([&, view](const worker::ComponentUpdateOp<Material>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewMaterialConfiguration configuration;
		shovelerViewEntityGetMaterialConfiguration(entity, &configuration);

		if(op.Update.type()) {
			configuration.type = convertMaterialType(*op.Update.type());
		}

        if(op.Update.texture()) {
            configuration.textureEntityId = **op.Update.texture();
        }

        if(op.Update.texture_sampler()) {
            configuration.textureSamplerEntityId = **op.Update.texture_sampler();
        }
        
        if(op.Update.tilemap()) {
            configuration.tilemapEntityId = **op.Update.tilemap();
        }
        
        if(op.Update.canvas()) {
            configuration.canvasEntityId = **op.Update.canvas();
        }
        
        if(op.Update.chunk()) {
            configuration.chunkEntityId = **op.Update.chunk();
        }

		if(op.Update.color()) {
			Vector3 color = op.Update.color()->value_or(Vector3{0, 0, 0});
			configuration.color = shovelerVector3(color.x(), color.y(), color.z());
		}

		if(op.Update.canvas_region_position()) {
			Vector2 position = op.Update.canvas_region_position()->value_or(Vector2{0.0f, 0.0f});
			configuration.canvasRegionPosition = shovelerVector2(position.x(), position.y());
		}

		if(op.Update.canvas_region_size()) {
			Vector2 size = op.Update.canvas_region_size()->value_or(Vector2{0.0f, 0.0f});
			configuration.canvasRegionSize = shovelerVector2(size.x(), size.y());
		}

		shovelerViewEntityUpdateMaterial(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<Material>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveMaterial(entity);
	});
}

static ShovelerComponentMaterialType convertMaterialType(MaterialType type)
{
	switch(type) {
		case MaterialType::COLOR:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_COLOR;
		case MaterialType::TEXTURE:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_TEXTURE;
		case MaterialType::PARTICLE:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_PARTICLE;
		case MaterialType::TILEMAP:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_TILEMAP;
		case MaterialType::CANVAS:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_CANVAS;
		case MaterialType::CHUNK:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_CHUNK;
		case MaterialType::TILE_SPRITE:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_TILE_SPRITE;
		case MaterialType::TEXT:
			return SHOVELER_COMPONENT_MATERIAL_TYPE_TEXT;
	}
}
