#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "material.h"

extern "C" {
#include <shoveler/view/material.h>
#include <shoveler/log.h>
}

using shoveler::Color;
using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Vector2;

static ShovelerViewMaterialType convertMaterialType(MaterialType type);

void registerMaterialCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Material>([&, view](const worker::AddComponentOp<Material>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewMaterialConfiguration configuration;
		configuration.type = convertMaterialType(op.Data.type());

		Color color = op.Data.color().value_or(Color{0, 0, 0});
		configuration.color = shovelerVector3(color.r(), color.g(), color.b());

		configuration.dataEntityId = op.Data.data_entity_id().value_or(0);

		Vector2 position = op.Data.canvas_region_position().value_or(Vector2{0.0f, 0.0f});
		configuration.canvasRegionPosition = shovelerVector2(position.x(), position.y());
		Vector2 size = op.Data.canvas_region_size().value_or(Vector2{0.0f, 0.0f});
		configuration.canvasRegionSize = shovelerVector2(size.x(), size.y());

		shovelerViewEntityAddMaterial(entity, configuration);
	});

	dispatcher.OnComponentUpdate<Material>([&, view](const worker::ComponentUpdateOp<Material>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerViewMaterialConfiguration configuration = *shovelerViewEntityGetMaterialConfiguration(entity);

		if(op.Update.type()) {
			configuration.type = convertMaterialType(*op.Update.type());
		}

		if(op.Update.color()) {
			Color color = op.Update.color()->value_or(Color{0, 0, 0});
			configuration.color = shovelerVector3(color.r(), color.g(), color.b());
		}

		if(op.Update.data_entity_id()) {
			configuration.dataEntityId = op.Update.data_entity_id()->value_or(0);
		}

		if(op.Update.canvas_region_position()) {
			Vector2 position = op.Update.canvas_region_position()->value_or(Vector2{0.0f, 0.0f});
			configuration.canvasRegionPosition = shovelerVector2(position.x(), position.y());
		}

		if(op.Update.canvas_region_size()) {
			Vector2 size = op.Update.canvas_region_size()->value_or(Vector2{0.0f, 0.0f});
			configuration.canvasRegionSize = shovelerVector2(size.x(), size.y());
		}

		shovelerViewEntityUpdateMaterial(entity, configuration);
	});

	dispatcher.OnRemoveComponent<Material>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveMaterial(entity);
	});
}

static ShovelerViewMaterialType convertMaterialType(MaterialType type)
{
	switch(type) {
		case MaterialType::COLOR:
			return SHOVELER_VIEW_MATERIAL_TYPE_COLOR;
		case MaterialType::TEXTURE:
			return SHOVELER_VIEW_MATERIAL_TYPE_TEXTURE;
		case MaterialType::PARTICLE:
			return SHOVELER_VIEW_MATERIAL_TYPE_PARTICLE;
		case MaterialType::TILEMAP:
			return SHOVELER_VIEW_MATERIAL_TYPE_TILEMAP;
		case MaterialType::CANVAS:
			return SHOVELER_VIEW_MATERIAL_TYPE_CANVAS;
		case MaterialType::CHUNK:
			return SHOVELER_VIEW_MATERIAL_TYPE_CHUNK;
	}
}
