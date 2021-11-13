#include "interest.h"

#include <stdlib.h> // malloc free

#include <glib.h>
#include <shoveler/component/sprite.h>
#include <shoveler/component/model.h>
#include <shoveler/component/light.h>
#include <shoveler/component/tilemap_tiles.h>
#include <shoveler/entity_component_id.h>
#include <shoveler/log.h>
#include <shoveler/spatialos_schema.h>
#include <shoveler/world.h>

#include "spatialos_client_schema.h"

static void freeComponentSet(void *componentSetPointer);

int shovelerClientComputeWorldInterest(ShovelerWorld *world, long long int clientEntityId, bool useAbsoluteConstraint, ShovelerVector3 absolutePosition, double viewDistance, Schema_Object *outputComponentSetInterest)
{
	// map from entity id to set of component IDs
	GHashTable *dependencies = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, freeComponentSet);

	GHashTableIter iter;
    ShovelerEntityComponentId *dependencyTarget;
	GQueue *dependencySourceList;
    g_hash_table_iter_init(&iter, world->reverseDependencies);
	while(g_hash_table_iter_next(&iter, (gpointer *) &dependencyTarget, (gpointer *) &dependencySourceList)) {
		int componentId = shovelerClientResolveComponentSchemaId(dependencyTarget->componentTypeId);
		if(componentId == 0) {
			shovelerLogWarning(
				"Found a dependency on component '%s' of entity %lld, but the component ID map doesn't contain an entry for this target, ignoring dependency.",
				dependencyTarget->componentTypeId,
				dependencyTarget->entityId);
			continue;
		}

		if(g_queue_get_length(dependencySourceList) == 0) {
			// nothing actually depends on this target
			continue;
		}

		GHashTable *componentSet = g_hash_table_lookup(dependencies, &dependencyTarget->entityId);
		if(componentSet == NULL) {
			componentSet = g_hash_table_new_full(g_int_hash, g_int_equal, free, NULL);

			long long int *entityIdStorage = malloc(sizeof(long long int));
			*entityIdStorage = dependencyTarget->entityId;
			g_hash_table_insert(dependencies, entityIdStorage, componentSet);
		}

		int *componentIdStorage = malloc(sizeof(int));
		*componentIdStorage = componentId;
		g_hash_table_add(componentSet, componentIdStorage);
	}

	int numQueries = 0;

	long long int *entityId;
	GHashTable *componentSet;
	g_hash_table_iter_init(&iter, dependencies);
	while(g_hash_table_iter_next(&iter, (gpointer *) &entityId, (gpointer *) &componentSet)) {
		Schema_Object *query = shovelerWorkerSchemaAddImprobableInterestComponentQuery(outputComponentSetInterest);
		shovelerWorkerSchemaSetImprobableInterestQueryEntityIdConstraint(query, *entityId);

		// always depend on Metadata
		shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(query, shovelerWorkerSchemaComponentIdImprobableMetadata);

		GHashTableIter componentSetIter;
		int *componentId;
		g_hash_table_iter_init(&componentSetIter, componentSet);
		while(g_hash_table_iter_next(&componentSetIter, (gpointer *) &componentId, NULL)) {
			shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(query, *componentId);
		}

		numQueries++;
	}

	if(useAbsoluteConstraint) {
		Schema_Object *query = shovelerWorkerSchemaAddImprobableInterestComponentQuery(outputComponentSetInterest);
		shovelerWorkerSchemaSetImprobableInterestQueryBoxConstraint(
			query,
			/* centerX */ absolutePosition.values[0],
			/* centerY */ absolutePosition.values[1],
			/* centerZ */ absolutePosition.values[2],
			/* edgeLengthX */ viewDistance,
			/* edgeLengthY */ 9999,
			/* edgeLengthZ */ viewDistance);
		shovelerWorkerSchemaAddImprobableInterestQueryResultComponentSetId(query, shovelerWorkerSchemaComponentSetIdClientPlayerSpatialInterest);
		numQueries++;
	} else {
		Schema_Object *query = shovelerWorkerSchemaAddImprobableInterestComponentQuery(outputComponentSetInterest);
		shovelerWorkerSchemaSetImprobableInterestQueryRelativeBoxConstraint(
			query,
			/* edgeLengthX */ viewDistance,
			/* edgeLengthY */ 9999,
			/* edgeLengthZ */ viewDistance);
		shovelerWorkerSchemaAddImprobableInterestQueryResultComponentSetId(query, shovelerWorkerSchemaComponentSetIdClientPlayerSpatialInterest);
		numQueries++;
	}

	Schema_Object *query = shovelerWorkerSchemaAddImprobableInterestComponentQuery(outputComponentSetInterest);
	shovelerWorkerSchemaSetImprobableInterestQuerySelfConstraint(query);
	shovelerWorkerSchemaAddImprobableInterestQueryResultComponentId(query, shovelerWorkerSchemaComponentIdClientHeartbeatPong);
	numQueries++;

	g_hash_table_destroy(dependencies);

	return numQueries;
}

static void freeComponentSet(void *componentSetPointer)
{
	GHashTable *componentSet = (GHashTable *) componentSetPointer;

	g_hash_table_destroy(componentSet);
}
