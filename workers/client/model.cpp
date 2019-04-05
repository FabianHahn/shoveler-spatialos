#include <shoveler.h>

#include "model.h"

extern "C" {
#include <shoveler/view/model.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
}

using shoveler::Model;
using shoveler::PolygonMode;

static GLuint getPolygonMode(PolygonMode polygonMode);

void registerModelCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Model>([&, view](const worker::AddComponentOp<Model>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewModelConfiguration modelConfiguration;
		modelConfiguration.drawableEntityId = op.Data.drawable_entity_id() != 0 ? op.Data.drawable_entity_id() : op.EntityId;
		modelConfiguration.materialEntityId = op.Data.material_entity_id() != 0 ? op.Data.material_entity_id() : op.EntityId;
		modelConfiguration.rotation = ShovelerVector3{op.Data.rotation().x(), op.Data.rotation().y(), op.Data.rotation().z()};
		modelConfiguration.scale = ShovelerVector3{op.Data.scale().x(), op.Data.scale().y(), op.Data.scale().z()};
		modelConfiguration.visible = op.Data.visible();
		modelConfiguration.emitter = op.Data.emitter();
		modelConfiguration.screenspace = op.Data.screenspace();
		modelConfiguration.castsShadow = op.Data.casts_shadow();
		modelConfiguration.polygonMode = getPolygonMode(op.Data.polygon_mode());
		shovelerViewEntityAddModel(entity, modelConfiguration);
	});

	dispatcher.OnComponentUpdate<Model>([&, view](const worker::ComponentUpdateOp<Model>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		if(op.Update.drawable_entity_id()) {
			shovelerViewEntityUpdateModelDrawableEntityId(entity, *op.Update.drawable_entity_id());
		}

		if(op.Update.material_entity_id()) {
			shovelerViewEntityUpdateModelMaterialEntityId(entity, *op.Update.material_entity_id());
		}

		if(op.Update.rotation()) {
			ShovelerVector3 rotation{op.Update.rotation()->x(), op.Update.rotation()->y(), op.Update.rotation()->z()};
			shovelerViewEntityUpdateModelRotation(entity, rotation);
		}

		if(op.Update.scale()) {
			ShovelerVector3 scale{-op.Update.scale()->x(), op.Update.scale()->y(), op.Update.scale()->z()};
			shovelerViewEntityUpdateModelScale(entity, scale);
		}

		if(op.Update.visible()) {
			shovelerViewEntityUpdateModelVisible(entity, *op.Update.visible());
		}

		if(op.Update.emitter()) {
			shovelerViewEntityUpdateModelEmitter(entity, *op.Update.emitter());
		}

		if(op.Update.screenspace()) {
			shovelerViewEntityUpdateModelScreenspace(entity, *op.Update.screenspace());
		}

		if(op.Update.polygon_mode()) {
			GLuint polygonMode = getPolygonMode(*op.Update.polygon_mode());
			shovelerViewEntityUpdateModelPolygonMode(entity, polygonMode);
		}
	});

	dispatcher.OnRemoveComponent<Model>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveModel(entity);
	});
}

static GLuint getPolygonMode(PolygonMode polygonMode)
{
	switch(polygonMode) {
		case PolygonMode::FILL:
			return GL_FILL;
		case PolygonMode::LINE:
			return GL_LINE;
		case PolygonMode::POINT:
			return GL_POINT;
		default:
			shovelerLogWarning("Tried to retrieve invalid polygon mode %d, defaulting to FILL.", polygonMode);
			return GL_FILL;
	}
}
