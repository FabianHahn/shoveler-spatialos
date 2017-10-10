#include <shoveler.h>
#include <improbable/standard_library.h>
#include <iostream>
#include <unordered_map>

using shoveler::Client;
using shoveler::Color;
using shoveler::Drawable;
using shoveler::DrawableType;
using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Model;
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

	const worker::ComponentRegistry& components = worker::Components<
		shoveler::Client,
		shoveler::Model,
		improbable::EntityAcl,
		improbable::Metadata,
		improbable::Persistence,
		improbable::Position>{};

	Color grayColor{0.7f, 0.7f, 0.7f};
	Drawable cubeDrawable{DrawableType::CUBE};
	Drawable quadDrawable{DrawableType::QUAD};
	Material grayColorMaterial{MaterialType::COLOR, grayColor, ""};
	worker::Map<std::uint32_t, WorkerRequirementSet> emptyComponentAclMap;

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
	planeEntity.Add<Model>({quadDrawable, grayColorMaterial});
	EntityAclData planeEntityAclData(clientRequirementSet, emptyComponentAclMap);
	planeEntity.Add<EntityAcl>({clientRequirementSet, emptyComponentAclMap});
	entities[2] = planeEntity;

	worker::Entity cubeEntity;
	cubeEntity.Add<Metadata>({"cube"});
	cubeEntity.Add<Persistence>({});
	cubeEntity.Add<Position>({{0, 0, 5}});
	cubeEntity.Add<Model>({cubeDrawable, grayColorMaterial});
	cubeEntity.Add<EntityAcl>({clientRequirementSet, emptyComponentAclMap});
	entities[3] = cubeEntity;

	worker::SaveSnapshot(components, path, entities);
	return 0;
}
