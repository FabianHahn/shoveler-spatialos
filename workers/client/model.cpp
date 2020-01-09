#include <shoveler.h>

#include "model.h"

extern "C" {
#include <shoveler/view/model.h>
#include <shoveler/component.h>
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

		ShovelerViewModelConfiguration configuration;
		configuration.positionEntityId = op.Data.position();
		configuration.drawableEntityId = op.Data.drawable();
		configuration.materialEntityId = op.Data.material();
        configuration.rotation = ShovelerVector3{op.Data.rotation().x(), op.Data.rotation().y(), op.Data.rotation().z()};
        configuration.scale = ShovelerVector3{op.Data.scale().x(), op.Data.scale().y(), op.Data.scale().z()};
        configuration.visible = op.Data.visible();
        configuration.emitter = op.Data.emitter();
        configuration.castsShadow = op.Data.casts_shadow();
        configuration.polygonMode = getPolygonMode(op.Data.polygon_mode());
		shovelerViewEntityAddModel(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Model>([&, view](const worker::ComponentUpdateOp<Model>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
        ShovelerComponent *component = shovelerViewEntityGetModelComponent(entity);

        if(op.Update.position()) {
			shovelerComponentUpdateCanonicalConfigurationOptionEntityId(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_POSITION, *op.Update.position());
        }

        if(op.Update.drawable()) {
			shovelerComponentUpdateCanonicalConfigurationOptionEntityId(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_DRAWABLE, *op.Update.drawable());
		}

        if(op.Update.material()) {
			shovelerComponentUpdateCanonicalConfigurationOptionEntityId(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_MATERIAL, *op.Update.material());
        }

		if(op.Update.rotation()) {
            shovelerComponentUpdateCanonicalConfigurationOptionVector3(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_ROTATION, ShovelerVector3{op.Update.rotation()->x(), op.Update.rotation()->y(), op.Update.rotation()->z()});
		}

        if(op.Update.scale()) {
            shovelerComponentUpdateCanonicalConfigurationOptionVector3(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_SCALE, ShovelerVector3{op.Update.scale()->x(), op.Update.scale()->y(), op.Update.scale()->z()});
        }

		if(op.Update.visible()) {
            shovelerComponentUpdateCanonicalConfigurationOptionBool(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_VISIBLE, *op.Update.visible());
		}

        if(op.Update.emitter()) {
            shovelerComponentUpdateCanonicalConfigurationOptionBool(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_EMITTER, *op.Update.emitter());
        }

        if(op.Update.casts_shadow()) {
            shovelerComponentUpdateCanonicalConfigurationOptionBool(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_CASTS_SHADOW, *op.Update.casts_shadow());
        }

		if(op.Update.polygon_mode()) {
            shovelerComponentUpdateCanonicalConfigurationOptionInt(component, SHOVELER_COMPONENT_MODEL_OPTION_ID_POLYGON_MODE, getPolygonMode(*op.Update.polygon_mode()));
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
