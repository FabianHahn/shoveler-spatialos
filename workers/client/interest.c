#include "interest.h"

#include <stdlib.h> // malloc free

#include <glib.h>
#include <shoveler/component/sprite.h>
#include <shoveler/component/model.h>
#include <shoveler/component/light.h>
#include <shoveler/component/tilemap_tiles.h>
#include <shoveler/log.h>
#include <shoveler/schema.h>

#include "schema.h"

static void freeComponentSet(void *componentSetPointer);

int shovelerClientComputeViewInterest(ShovelerView *view, long long int clientEntityId, bool useAbsoluteConstraint, ShovelerVector3 absolutePosition, double viewDistance, Schema_Object *outputComponentInterest)
{
	// map from entity id to set of component IDs
	GHashTable *dependencies = g_hash_table_new_full(g_int64_hash, g_int64_equal, free, freeComponentSet);

	GHashTableIter iter;
	ShovelerViewQualifiedComponent *dependencyTarget;
	GQueue *dependencySourceList;
	g_hash_table_iter_init(&iter, view->reverseDependencies);
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
		Schema_Object *query = Schema_AddObject(outputComponentInterest, shovelerWorkerSchemaImprobableComponentInterestFieldIdQueries);
		Schema_Object *constraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);

		// entity ID constraint
		Schema_AddEntityId(constraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdEntityIdConstraint, *entityId);

		// always depend on Metadata
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdImprobableMetadata);

		GHashTableIter componentSetIter;
		int *componentId;
		g_hash_table_iter_init(&componentSetIter, componentSet);
		while(g_hash_table_iter_next(&componentSetIter, (gpointer *) &componentId, NULL)) {
			Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, *componentId);
		}

		numQueries++;
	}

	if(useAbsoluteConstraint) {
		Schema_Object *query = Schema_AddObject(outputComponentInterest, shovelerWorkerSchemaImprobableComponentInterestFieldIdQueries);
		Schema_Object *constraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);
		Schema_Object *absoluteConstraint = Schema_AddObject(constraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdBoxConstraint);

		Schema_Object *center = Schema_AddObject(absoluteConstraint, shovelerWorkerSchemaImprobableComponentInterestBoxConstraintFieldIdCenter);
		Schema_AddDouble(center, shovelerWorkerSchemaImprobableCoordinatesFieldIdX, absolutePosition.values[0]);
		Schema_AddDouble(center, shovelerWorkerSchemaImprobableCoordinatesFieldIdY, absolutePosition.values[1]);
		Schema_AddDouble(center, shovelerWorkerSchemaImprobableCoordinatesFieldIdZ, absolutePosition.values[2]);

		Schema_Object *edgeLength = Schema_AddObject(absoluteConstraint, shovelerWorkerSchemaImprobableComponentInterestBoxConstraintFieldIdEdgeLength);
		Schema_AddDouble(edgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdX, viewDistance);
		Schema_AddDouble(edgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdY, 9999);
		Schema_AddDouble(edgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdZ, viewDistance);

		// result component IDs
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdLight);
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdModel);
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdSprite);
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdTilemapTiles);

		numQueries++;
	} else {
		Schema_Object *query = Schema_AddObject(outputComponentInterest, shovelerWorkerSchemaImprobableComponentInterestFieldIdQueries);
		Schema_Object *constraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);
		Schema_Object *relativeConstraint = Schema_AddObject(constraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdRelativeBoxConstraint);

		Schema_Object *edgeLength = Schema_AddObject(relativeConstraint, shovelerWorkerSchemaImprobableComponentInterestRelativeBoxConstraintFieldIdEdgeLength);
		Schema_AddDouble(edgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdX, viewDistance);
		Schema_AddDouble(edgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdY, 9999);
		Schema_AddDouble(edgeLength, shovelerWorkerSchemaImprobableEdgeLengthFieldIdZ, viewDistance);

		// result component IDs
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdLight);
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdModel);
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdSprite);
		Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdTilemapTiles);

		numQueries++;
	}

	Schema_Object *query = Schema_AddObject(outputComponentInterest, shovelerWorkerSchemaImprobableComponentInterestFieldIdQueries);
	Schema_Object *constraint = Schema_AddObject(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdConstraint);
	Schema_AddInt64(constraint, shovelerWorkerSchemaImprobableComponentInterestQueryConstraintFieldIdEntityIdConstraint, clientEntityId);

	// result component IDs
	Schema_AddUint32(query, shovelerWorkerSchemaImprobableComponentInterestQueryFieldIdResultComponentId, shovelerWorkerSchemaComponentIdClientHeartbeatPong);

	numQueries++;


	g_hash_table_destroy(dependencies);

	return numQueries;
}

static void freeComponentSet(void *componentSetPointer)
{
	GHashTable *componentSet = (GHashTable *) componentSetPointer;

	g_hash_table_destroy(componentSet);
}
