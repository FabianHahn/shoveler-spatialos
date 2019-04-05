#include <improbable/standard_library.h>

#include "metadata.h"

extern "C" {
#include <shoveler/log.h>
}

using improbable::Metadata;

void registerMetadataCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Metadata>([&, view](const worker::AddComponentOp<Metadata>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntitySetType(entity, op.Data.entity_type().c_str());
	});

	dispatcher.OnComponentUpdate<Metadata>([&, view](const worker::ComponentUpdateOp<Metadata>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		if(op.Update.entity_type()) {
			shovelerViewEntitySetType(entity, op.Update.entity_type()->c_str());
		}
	});

	dispatcher.OnRemoveComponent<Metadata>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntitySetType(entity, NULL);
	});
}
