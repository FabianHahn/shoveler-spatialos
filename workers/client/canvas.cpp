#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tileset.h"

extern "C" {
#include <shoveler/view/canvas.h>
#include <shoveler/component.h>
#include <shoveler/log.h>
}

using shoveler::Canvas;
using worker::EntityId;
using worker::List;

void registerCanvasCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Canvas>([&, view](const worker::AddComponentOp<Canvas>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewCanvasConfiguration configuration;
		configuration.numLayers = op.Data.num_layers();

		shovelerViewEntityAddCanvas(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Canvas>([&, view](const worker::ComponentUpdateOp<Canvas>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerComponent *component = shovelerViewEntityGetCanvasComponent(entity);

        ShovelerViewCanvasConfiguration configuration;
		shovelerViewEntityGetCanvasConfiguration(entity, &configuration);

		if(op.Update.num_layers()) {
			configuration.numLayers = *op.Update.num_layers();
		}

		shovelerViewEntityUpdateCanvas(entity, &configuration);
		shovelerComponentActivate(component);
	});

	dispatcher.OnRemoveComponent<Canvas>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveCanvas(entity);
	});
}
