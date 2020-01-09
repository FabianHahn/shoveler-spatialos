#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "resource.h"

extern "C" {
#include <shoveler/view/drawable.h>
#include <shoveler/log.h>
}

using shoveler::Drawable;
using shoveler::DrawableType;

static ShovelerComponentDrawableType convertDrawableType(DrawableType type);

void registerDrawableCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Drawable>([&, view](const worker::AddComponentOp<Drawable>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewDrawableConfiguration configuration;
		configuration.type = convertDrawableType(op.Data.type());

		shovelerViewEntityAddDrawable(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Drawable>([&, view](const worker::ComponentUpdateOp<Drawable>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerViewDrawableConfiguration configuration;
		shovelerViewEntityGetDrawableConfiguration(entity, &configuration);

		if(op.Update.type()) {
			configuration.type = convertDrawableType(*op.Update.type());
		}

		shovelerViewEntityUpdateDrawable(entity, &configuration);
	});

	dispatcher.OnRemoveComponent<Drawable>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveDrawable(entity);
	});
}

static ShovelerComponentDrawableType convertDrawableType(DrawableType type)
{
	switch(type) {
		case DrawableType::CUBE:
			return SHOVELER_COMPONENT_DRAWABLE_TYPE_CUBE;
		case DrawableType::QUAD:
			return SHOVELER_COMPONENT_DRAWABLE_TYPE_QUAD;
		case DrawableType::POINT:
			return SHOVELER_COMPONENT_DRAWABLE_TYPE_POINT;
	}
}
