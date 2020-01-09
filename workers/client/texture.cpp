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
		configuration.imageResourceEntityId = op.Data.image_resource();

		shovelerViewEntityAddTexture(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Texture>([&, view](const worker::ComponentUpdateOp<Texture>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTextureConfiguration configuration;
		shovelerViewEntityGetTextureConfiguration(entity, &configuration);

		if(op.Update.image_resource()) {
			configuration.imageResourceEntityId = *op.Update.image_resource();
		}

		shovelerViewEntityUpdateTexture(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<Texture>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTexture(entity);
	});
}
