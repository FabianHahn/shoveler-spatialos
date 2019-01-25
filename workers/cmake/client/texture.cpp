#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "resource.h"

extern "C" {
#include <shoveler/view/texture.h>
#include <shoveler/log.h>
}

using shoveler::Texture;

void registerTextureCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Texture>([&, view](const worker::AddComponentOp<Texture>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTextureConfiguration configuration;
		configuration.imageResourceEntityId = op.Data.image_resource_entity_id();
		configuration.interpolate = op.Data.interpolate();
		configuration.useMipmaps = op.Data.use_mipmaps();
		configuration.clamp = op.Data.clamp();

		shovelerViewEntityAddTexture(entity, configuration);
	});

	dispatcher.OnComponentUpdate<Texture>([&, view](const worker::ComponentUpdateOp<Texture>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerViewTextureConfiguration configuration = *shovelerViewEntityGetTextureConfiguration(entity);

		if(op.Update.image_resource_entity_id()) {
			configuration.imageResourceEntityId = *op.Update.image_resource_entity_id();
		}

		if(op.Update.interpolate()) {
			configuration.imageResourceEntityId = *op.Update.interpolate();
		}

		if(op.Update.use_mipmaps()) {
			configuration.useMipmaps = *op.Update.use_mipmaps();
		}

		if(op.Update.clamp()) {
			configuration.imageResourceEntityId = *op.Update.clamp();
		}

		shovelerViewEntityUpdateTexture(entity, configuration);
	});

	dispatcher.OnRemoveComponent<Texture>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTexture(entity);
	});
}
