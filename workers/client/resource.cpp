#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "resource.h"

extern "C" {
#include <shoveler/view/resource.h>
#include <shoveler/component.h>
#include <shoveler/log.h>
}

using shoveler::Resource;

void registerResourceCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Resource>([&, view](const worker::AddComponentOp<Resource>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewResourceConfiguration configuration;
		configuration.buffer = reinterpret_cast<const unsigned char *>(op.Data.buffer().c_str());
		configuration.bufferSize = op.Data.buffer().size();

		shovelerViewEntityAddResource(entity, &configuration);
	});

	dispatcher.OnComponentUpdate<Resource>([&, view](const worker::ComponentUpdateOp<Resource>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerComponent *component = shovelerViewEntityGetResourceComponent(entity);

		if(op.Update.buffer()) {
			ShovelerViewResourceConfiguration configuration;
			configuration.buffer = new unsigned char[op.Update.buffer()->size()];
			memcpy(const_cast<unsigned char *>(configuration.buffer), reinterpret_cast<const unsigned char *>(op.Update.buffer()->c_str()), op.Update.buffer()->size());
			configuration.bufferSize = op.Update.buffer()->size();

			shovelerComponentUpdateCanonicalConfigurationOptionBytes(component, SHOVELER_COMPONENT_RESOURCE_OPTION_ID_BUFFER, configuration.buffer, configuration.bufferSize);
			delete[] configuration.buffer;
		}
	});

	dispatcher.OnRemoveComponent<Resource>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveResource(entity);
	});
}

