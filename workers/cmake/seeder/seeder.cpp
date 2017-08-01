#include <shoveler.h>
#include <improbable/standard_library.h>
#include <iostream>
#include <unordered_map>

using shoveler::Client;
using shoveler::Plane;
using improbable::EntityAcl;
using improbable::EntityAclData;
using improbable::Metadata;
using improbable::Persistence;
using improbable::Position;
using improbable::WorkerAttributeSet;
using improbable::WorkerRequirementSet;

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <seed snapshot file>" << std::endl;
		return 1;
	}

	std::string path(argv[1]);

	WorkerAttributeSet clientAttributeSet({"client"});
	WorkerRequirementSet clientRequirementSet({clientAttributeSet});

	std::unordered_map<worker::EntityId, worker::Entity> entities;

	worker::Entity bootstrapEntity;
	bootstrapEntity.Add<Metadata>({"bootstrap"});
	bootstrapEntity.Add<Persistence>({});
	bootstrapEntity.Add<Position>({{0, 0, 0}});
	bootstrapEntity.Add<Client>({});
	worker::Map<std::uint32_t, WorkerRequirementSet> bootstrapComponentAclMap;
	bootstrapComponentAclMap.insert({{Client::ComponentId, clientRequirementSet}});
	EntityAclData bootstrapEntityAclData(clientRequirementSet, bootstrapComponentAclMap);
	bootstrapEntity.Add<EntityAcl>(bootstrapEntityAclData);
	entities[1] = bootstrapEntity;

	worker::Entity planeEntity;
	planeEntity.Add<Metadata>({"plane"});
	planeEntity.Add<Persistence>({});
	planeEntity.Add<Position>({{0, 0, 0}});
	planeEntity.Add<Plane>({{0.5, 0.5, 0.5}, 10.0});
	worker::Map<std::uint32_t, WorkerRequirementSet> planeComponentAclMap;
	EntityAclData planeEntityAclData(clientRequirementSet, planeComponentAclMap);
	planeEntity.Add<EntityAcl>(planeEntityAclData);
	entities[2] = planeEntity;

	worker::SaveSnapshot(path, entities);
	return 0;
}
