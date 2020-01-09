#include <cstring> // memcpy strdup

#include <shoveler.h>

#include "tilemap_colliders.h"

extern "C" {
#include <shoveler/view/tilemap_colliders.h>
#include <shoveler/log.h>
}

using shoveler::TilemapColliders;
using worker::EntityId;
using worker::List;

void registerTilemapCollidersCallbacks(worker::Dispatcher& dispatcher, ShovelerView *view)
{
	dispatcher.OnAddComponent<TilemapColliders>([&, view](const worker::AddComponentOp<TilemapColliders>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

		ShovelerViewTilemapCollidersConfiguration configuration;
		configuration.numColumns = op.Data.num_columns();
		configuration.numRows = op.Data.num_rows();

		unsigned char *colliders = new unsigned char[configuration.numColumns * configuration.numRows];

        const std::string& collidersString = op.Data.colliders();

        for(int row = 0; row < configuration.numRows; ++row) {
			for(int column = 0; column < configuration.numColumns; ++column) {
				int tileIndex = row * configuration.numColumns + column;
                colliders[tileIndex] = collidersString[tileIndex];
			}
		}
        configuration.colliders = colliders;

		shovelerViewEntityAddTilemapColliders(entity, &configuration);

		delete[] colliders;
	});

	dispatcher.OnComponentUpdate<TilemapColliders>([&, view](const worker::ComponentUpdateOp<TilemapColliders>& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);

        ShovelerViewTilemapCollidersConfiguration configuration;
		shovelerViewEntityGetTilemapCollidersConfiguration(entity, &configuration);

		if(op.Update.num_columns()) {
			configuration.numColumns = *op.Update.num_columns();
		}

		if(op.Update.num_rows()) {
			configuration.numRows = *op.Update.num_rows();
		}

        unsigned char *colliders = NULL;
        if(op.Update.colliders()) {
            const std::string& collidersString = *op.Update.colliders();

            colliders = new unsigned char[configuration.numColumns * configuration.numRows];
            for(int row = 0; row < configuration.numRows; ++row) {
                for(int column = 0; column < configuration.numColumns; ++column) {
                    int tileIndex = row * configuration.numColumns + column;
                    colliders[tileIndex] = collidersString[tileIndex];
                }
            }
            configuration.colliders = colliders;
        }

        shovelerViewEntityUpdateTilemapColliders(entity, &configuration);

        if(colliders != NULL) {
            delete[] colliders;
        }
	});

	dispatcher.OnRemoveComponent<TilemapColliders>([&, view](const worker::RemoveComponentOp& op) {
		ShovelerViewEntity *entity = shovelerViewGetEntity(view, op.EntityId);
		shovelerViewEntityRemoveTilemapColliders(entity);
	});
}
