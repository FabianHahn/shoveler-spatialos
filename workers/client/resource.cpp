#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "resource.h"

extern "C" {
#include <shoveler/view/resources.h>
#include <shoveler/log.h>
}

using shoveler::Resource;

void registerResourceCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<Resource>([&, view](const worker::AddComponentOp<Resource>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewResourceConfiguration configuration;
		configuration.typeId = op.Data.type_id().c_str();
		configuration.buffer = reinterpret_cast<const unsigned char *>(op.Data.content().c_str());
		configuration.bufferSize = op.Data.content().size();

		shovelerViewEntityAddResource(entity, configuration);
	});

	dispatcher.OnComponentUpdate<Resource>([&, view](const worker::ComponentUpdateOp<Resource>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		ShovelerViewResourceConfiguration currentConfiguration;
		shovelerViewEntityGetResourceConfiguration(entity, &currentConfiguration);

		ShovelerViewResourceConfiguration configuration;
		if(op.Update.type_id()) {
			configuration.typeId = strdup(op.Update.type_id()->c_str());
		} else {
			configuration.typeId = strdup(currentConfiguration.typeId);
		}

		if(op.Update.content()) {
			configuration.buffer = new unsigned char[op.Update.content()->size()];
			memcpy(const_cast<unsigned char *>(configuration.buffer), reinterpret_cast<const unsigned char *>(op.Update.content()->c_str()), op.Update.content()->size());
			configuration.bufferSize = op.Update.content()->size();
		} else {
			configuration.buffer = new unsigned char[currentConfiguration.bufferSize];
			memcpy(const_cast<unsigned char *>(configuration.buffer), currentConfiguration.buffer, currentConfiguration.bufferSize);
			configuration.bufferSize = currentConfiguration.bufferSize;
		}

		shovelerViewEntityUpdateResourceConfiguration(entity, configuration);

		free(const_cast<char *>(configuration.typeId));
		delete[] configuration.buffer;
	});

	dispatcher.OnRemoveComponent<Resource>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveResource(entity);
	});
}

