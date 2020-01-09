#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "resource.h"

extern "C" {
#include <shoveler/view/sampler.h>
#include <shoveler/log.h>
}

using shoveler::Sampler;

void registerSamplerCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Sampler>([&, view](const worker::AddComponentOp<Sampler>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewSamplerConfiguration configuration;
		configuration.interpolate = op.Data.interpolate();
		configuration.useMipmaps = op.Data.use_mipmaps();
		configuration.clamp = op.Data.clamp();

		shovelerViewEntityAddSampler(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Sampler>([&, view](const worker::ComponentUpdateOp<Sampler>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewSamplerConfiguration configuration;
		shovelerViewEntityGetSamplerConfiguration(entity, &configuration);

		if(op.Update.interpolate()) {
			configuration.interpolate = *op.Update.interpolate();
		}

		if(op.Update.use_mipmaps()) {
			configuration.useMipmaps = *op.Update.use_mipmaps();
		}

		if(op.Update.clamp()) {
			configuration.clamp = *op.Update.clamp();
		}

		shovelerViewEntityUpdateSampler(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<Sampler>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveSampler(entity);
	});
}
