#include <shoveler.h>

#include "light.h"

extern "C" {
#include <shoveler/view/light.h>
#include <shoveler/log.h>
#include <shoveler/types.h>
}

using shoveler::Light;
using shoveler::LightType;

static ShovelerComponentLightType convertLightType(LightType type);

void registerLightCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Light>([&, view](const worker::AddComponentOp<Light>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewLightConfiguration lightConfiguration;
		lightConfiguration.positionEntityId = op.Data.position();
		lightConfiguration.type = convertLightType(op.Data.type());
		lightConfiguration.width = (int) op.Data.width();
		lightConfiguration.height = (int) op.Data.height();
		lightConfiguration.samples = (GLsizei) op.Data.samples();
		lightConfiguration.ambientFactor = op.Data.ambient_factor();
		lightConfiguration.exponentialFactor = op.Data.exponential_factor();
		lightConfiguration.color = ShovelerVector3{op.Data.color().x(), op.Data.color().y(), op.Data.color().z()};
		shovelerViewEntityAddLight(entity, &lightConfiguration);
	});

	dispatcher.OnComponentUpdate<Light>([&, view](const worker::ComponentUpdateOp<Light>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewLightConfiguration configuration;
		shovelerViewEntityGetLightConfiguration(entity, &configuration);

        if(op.Update.position()) {
            configuration.positionEntityId = *op.Update.position();
        }

		if(op.Update.type()) {
			configuration.type = convertLightType(*op.Update.type());
		}

		if(op.Update.width()) {
			configuration.width = (int) *op.Update.width();
		}

		if(op.Update.height()) {
			configuration.height = (int) *op.Update.height();
		}

		if(op.Update.samples()) {
			configuration.samples = (GLsizei) *op.Update.samples();
		}

		if(op.Update.ambient_factor()) {
			configuration.ambientFactor = *op.Update.ambient_factor();
		}

		if(op.Update.exponential_factor()) {
			configuration.exponentialFactor = *op.Update.exponential_factor();
		}

		if(op.Update.color()) {
			configuration.color = ShovelerVector3{op.Update.color()->x(), op.Update.color()->y(), op.Update.color()->z()};
		}

		shovelerViewEntityUpdateLight(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<Light>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveLight(entity);
	});
}

static ShovelerComponentLightType convertLightType(LightType type)
{
	switch(type) {
		case LightType::POINT:
			return SHOVELER_COMPONENT_LIGHT_TYPE_POINT;
		case LightType::SPOT:
			return SHOVELER_COMPONENT_LIGHT_TYPE_SPOT;
	}
}
