#include "interest.h"

#include <stdlib.h> // malloc free

#include <glib.h>
#include <shoveler/log.h>
#include <shoveler/component/sprite.h>
#include <shoveler/component/model.h>
#include <shoveler/component/light.h>
#include <shoveler/component/tilemap_tiles.h>

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
		Schema_Object *query = Schema_AddObject(outputComponentInterest, /* fieldId */ 1);
		Schema_Object *constraint = Schema_AddObject(query, /* fieldId */ 1);

		// entity ID constraint
		Schema_AddEntityId(constraint, /* fieldId */ 7, *entityId);

		// always depend on EntityAcl and Metadata
		Schema_AddUint32(query, /* fieldId */ 3, /* value */ 50);
		Schema_AddUint32(query, /* fieldId */ 3, /* value */ 53);

		GHashTableIter componentSetIter;
		int *componentId;
		g_hash_table_iter_init(&componentSetIter, componentSet);
		while(g_hash_table_iter_next(&componentSetIter, (gpointer *) &componentId, NULL)) {
			Schema_AddUint32(query, /* fieldId */ 3, *componentId);
		}

		numQueries++;
	}

	if(useAbsoluteConstraint) {
		Schema_Object *query = Schema_AddObject(outputComponentInterest, /* fieldId */ 1);
		Schema_Object *constraint = Schema_AddObject(query, /* fieldId */ 1);
		Schema_Object *absoluteConstraint = Schema_AddObject(constraint, /* fieldId */ 3);

		Schema_Object *center = Schema_AddObject(absoluteConstraint, /* fieldId */ 1);
		Schema_AddDouble(center, /* fieldId */ 1, absolutePosition.values[0]);
		Schema_AddDouble(center, /* fieldId */ 2, absolutePosition.values[1]);
		Schema_AddDouble(center, /* fieldId */ 3, absolutePosition.values[2]);

		Schema_Object *edgeLength = Schema_AddObject(absoluteConstraint, /* fieldId */ 2);
		Schema_AddDouble(edgeLength, /* fieldId */ 1, viewDistance);
		Schema_AddDouble(edgeLength, /* fieldId */ 2, 9999);
		Schema_AddDouble(edgeLength, /* fieldId */ 3, viewDistance);

		// result component IDs
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdLight));
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdModel));
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdSprite));
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdTilemapTiles));

		numQueries++;
	} else {
		Schema_Object *query = Schema_AddObject(outputComponentInterest, /* fieldId */ 1);
		Schema_Object *constraint = Schema_AddObject(query, /* fieldId */ 1);
		Schema_Object *relativeConstraint = Schema_AddObject(constraint, /* fieldId */ 6);

		Schema_Object *edgeLength = Schema_AddObject(relativeConstraint, /* fieldId */ 1);
		Schema_AddDouble(edgeLength, /* fieldId */ 1, viewDistance);
		Schema_AddDouble(edgeLength, /* fieldId */ 2, 9999);
		Schema_AddDouble(edgeLength, /* fieldId */ 3, viewDistance);

		// result component IDs
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdLight));
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdModel));
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdSprite));
		Schema_AddUint32(query, /* fieldId */ 3, shovelerClientResolveComponentSchemaId(shovelerComponentTypeIdTilemapTiles));

		numQueries++;
	}

	Schema_Object *query = Schema_AddObject(outputComponentInterest, /* fieldId */ 1);
	Schema_Object *constraint = Schema_AddObject(query, /* fieldId */ 1);
	Schema_AddInt64(constraint, /* fieldId */ 7, clientEntityId);

	// result component IDs
	Schema_AddUint32(query, /* fieldId */ 3, 13352); // ClientHeartbeatPong

	numQueries++;


	g_hash_table_destroy(dependencies);

	return numQueries;
}

static void freeComponentSet(void *componentSetPointer)
{
	GHashTable *componentSet = (GHashTable *) componentSetPointer;

	g_hash_table_destroy(componentSet);
}
