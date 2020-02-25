#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "image.h"

extern "C" {
#include <shoveler/view/image.h>
#include <shoveler/log.h>
}

using shoveler::Image;
using shoveler::ImageFormat;

static ShovelerComponentImageFormat convertImageFormat(ImageFormat format);

void registerImageCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Image>([&, view](const worker::AddComponentOp<Image>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewImageConfiguration configuration;
		configuration.format = convertImageFormat(op.Data.format());
		configuration.resourceEntityId = op.Data.resource();

		shovelerViewEntityAddImage(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Image>([&, view](const worker::ComponentUpdateOp<Image>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerViewImageConfiguration currentConfiguration;
		shovelerViewEntityGetImageConfiguration(entity, &currentConfiguration);

		ShovelerViewImageConfiguration configuration;
		if(op.Update.format()) {
			configuration.format = convertImageFormat(*op.Update.format());
		}

		if(op.Update.resource()) {
			configuration.resourceEntityId = *op.Update.resource();
		}

		shovelerViewEntityUpdateImageConfiguration(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<Image>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveImage(entity);
	});
}

static ShovelerComponentImageFormat convertImageFormat(ImageFormat format)
{
	switch(format) {
		case ImageFormat::PNG:
			return SHOVELER_COMPONENT_IMAGE_FORMAT_PNG;
		case ImageFormat::PPM:
		default:
			return SHOVELER_COMPONENT_IMAGE_FORMAT_PPM;
	}
}
