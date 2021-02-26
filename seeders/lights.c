#include <inttypes.h> // PRId64
#include <stdbool.h> // bool
#include <stdlib.h> // EXIT_FAILURE
#include <string.h> // strlen

#include <improbable/c_worker.h>
#include <improbable/c_schema.h>
#include <shoveler/constants.h>
#include <shoveler/log.h>
#include <shoveler/schema.h>

static const int64_t serverPartitionEntityId = 1;

static bool writeEntity(Worker_SnapshotOutputStream *snapshotOutputStream, Worker_Entity *entity);

int main(int argc, char **argv)
{
	shovelerLogInit("shoveler-spatialos/seeders/", SHOVELER_LOG_LEVEL_INFO_UP, stdout);

	if(argc != 2) {
		shovelerLogError("Usage:\n\t%s <seed snapshot file>", argv[0]);
		return EXIT_FAILURE;
	}

	const char *filename = argv[1];

	Worker_ComponentVtable componentVtable = {0};

	Worker_SnapshotParameters snapshotParameters = {0};
	snapshotParameters.snapshot_type = WORKER_SNAPSHOT_TYPE_BINARY;
	snapshotParameters.default_component_vtable = &componentVtable;

	Worker_SnapshotOutputStream *snapshotOutputStream = Worker_SnapshotOutputStream_Create(filename, &snapshotParameters);
	if (snapshotOutputStream == NULL) {
		shovelerLogError("Failed to create snapshot output stream to '%s'.", filename);
		return EXIT_FAILURE;
	}

	Worker_SnapshotState snapshotState = Worker_SnapshotOutputStream_GetState(snapshotOutputStream);
	if (snapshotState.stream_state != WORKER_STREAM_STATE_GOOD) {
		shovelerLogError("Snapshot output stream not in good state after opening: %s", snapshotState.error_message);
		return EXIT_FAILURE;
	}

	Worker_EntityId nextEntityId = 1;
	{ // bootstrap
		Worker_ComponentData componentData[7] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("bootstrap");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, 0.0, 0.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, 0.0f, 0.0f));
		componentData[4] = shovelerWorkerSchemaCreateBootstrapComponent();
		componentData[5] = shovelerWorkerSchemaCreateImprobableAuthorityDelegationComponent();
		shovelerWorkerSchemaAddImprobableAuthorityDelegation(&componentData[5], shovelerWorkerSchemaComponentSetIdServerBootstrapAuthority, serverPartitionEntityId);
		componentData[6] = shovelerWorkerSchemaCreateImprobableInterestComponent();
		Schema_Object *componentInterest = shovelerWorkerSchemaAddImprobableInterestForComponent(
			&componentData[6], shovelerWorkerSchemaComponentIdBootstrap);
		Schema_Object *query = shovelerWorkerSchemaAddImprobableInterestComponentQuery(componentInterest);
		shovelerWorkerSchemaSetImprobableInterestQueryComponentConstraint(query, shovelerWorkerSchemaComponentIdClient);
		shovelerWorkerSchemaSetImprobableInterestQueryFullSnapshotResult(query);

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId cubeDrawableEntityId = nextEntityId;
	{ // cube drawable
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("drawable");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, 0.0, 0.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, 0.0f, 0.0f));
		componentData[4] = shovelerWorkerSchemaCreateDrawableCubeComponent();

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId quadDrawableEntityId = nextEntityId;
	{ // quad drawable
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("drawable");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, 0.0, 0.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, 0.0f, 0.0f));
		componentData[4] = shovelerWorkerSchemaCreateDrawableQuadComponent();

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId pointDrawableEntityId = nextEntityId;
	{ // point drawable
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("drawable");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, 0.0, 0.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, 0.0f, 0.0f));
		componentData[4] = shovelerWorkerSchemaCreateDrawablePointComponent();

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId grayColorMaterialEntityId = nextEntityId;
	{ // gray color material
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("material");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, 0.0, 0.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, 0.0f, 0.0f));
		componentData[4] = shovelerWorkerSchemaCreateMaterialColorComponent(shovelerVector4(0.7f, 0.7f, 0.7f, 1.0f));

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_EntityId whiteParticleMaterialEntityId = nextEntityId;
	{ // white particle color material
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("material");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, 0.0, 0.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, 0.0f, 0.0f));
		componentData[4] = shovelerWorkerSchemaCreateMaterialParticleComponent(shovelerVector4(1.0f, 1.0f, 1.0f, 1.0f));

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	{ // plane
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("plane");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, -2.0, 0.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, -2.0f, 0.0f));
		componentData[4] = shovelerWorkerSchemaCreateModelComponent(
			/* position */ 0,
			quadDrawableEntityId,
			grayColorMaterialEntityId,
			/* rotation */ shovelerVector3(SHOVELER_PI / 2.0f, 0.0f, 0.0f),
			/* scale */ shovelerVector3(25.0f, 25.0f, 1.0f),
			/* visible */ true,
			/* emitter */ false,
			/* castsShadow */ true,
			/* polygonMode */ shovelerWorkerSchemaPolygonModeFill);

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	{ // cube
		Worker_ComponentData componentData[5] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("cube");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(0.0, 0.0, 5.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(0.0f, 0.0f, 5.0f));
		componentData[4] = shovelerWorkerSchemaCreateModelComponent(
			/* position */ 0,
			cubeDrawableEntityId,
			grayColorMaterialEntityId,
			/* rotation */ shovelerVector3(0.0f, 0.0f, 0.0f),
			/* scale */ shovelerVector3(1.0f, 1.0f, 1.0f),
			/* visible */ true,
			/* emitter */ false,
			/* castsShadow */ true,
			/* polygonMode */ shovelerWorkerSchemaPolygonModeFill);

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	{ // light
		Worker_ComponentData componentData[6] = {0};
		Worker_Entity entity;
		entity.entity_id = nextEntityId++;
		entity.component_count = sizeof(componentData) / sizeof(componentData[0]);
		entity.components = componentData;
		componentData[0] = shovelerWorkerSchemaCreateImprobableMetadataComponent("light");
		componentData[1] = shovelerWorkerSchemaCreateImprobablePersistenceComponent();
		componentData[2] = shovelerWorkerSchemaCreateImprobablePositionComponent(1.0, 5.0, -1.0);
		componentData[3] = shovelerWorkerSchemaCreatePositionComponent(shovelerVector3(-1.0f, 5.0f, -1.0f));
		componentData[4] = shovelerWorkerSchemaCreateModelComponent(
			/* position */ 0,
			pointDrawableEntityId,
			whiteParticleMaterialEntityId,
			/* rotation */ shovelerVector3(0.0f, 0.0f, 0.0f),
			/* scale */ shovelerVector3(0.5f, 0.5f, 0.0f),
			/* visible */ true,
			/* emitter */ true,
			/* castsShadow */ false,
			/* polygonMode */ shovelerWorkerSchemaPolygonModeFill);
		componentData[5] = shovelerWorkerSchemaCreateLightComponent(
			/* position */ 0,
			shovelerWorkerSchemaLightTypePoint,
			/* width */ 1024,
			/* height */ 1024,
			/* samples */ 1,
			/* ambientFactor */ 0.01f,
			/* exponentialFactor */ 80.0f,
			/* color */ shovelerVector3(1.0f, 1.0f, 1.0f));

		if(!writeEntity(snapshotOutputStream, &entity)) {
			return EXIT_FAILURE;
		}
	}

	Worker_SnapshotOutputStream_Destroy(snapshotOutputStream);
	shovelerLogInfo("Successfully wrote lights snapshot with %"PRId64" entities.", nextEntityId - 1);
}

static bool writeEntity(Worker_SnapshotOutputStream *snapshotOutputStream, Worker_Entity *entity)
{
	Worker_SnapshotOutputStream_WriteEntity(snapshotOutputStream, entity);
	const char *lastErrorMessage = Worker_SnapshotOutputStream_GetLastWarning(snapshotOutputStream);
	if (lastErrorMessage != NULL) {
		shovelerLogError("Failed to write entity %"PRId64" to snapshot output stream: %s", entity->entity_id, lastErrorMessage);
		return false;
	}

	Worker_SnapshotState snapshotState = Worker_SnapshotOutputStream_GetState(snapshotOutputStream);
	if (snapshotState.stream_state != WORKER_STREAM_STATE_GOOD) {
		shovelerLogError("Snapshot output stream not in good state after writing entity %"PRId64": %s", entity->entity_id, snapshotState.error_message);
		return EXIT_FAILURE;
	}

	return true;
}