#include <shoveler.h>
#include <improbable/standard_library.h>
#include <iostream>
#include <unordered_map>

using shoveler::Client;
using shoveler::Color;
using shoveler::Drawable;
using shoveler::DrawableType;
using shoveler::Light;
using shoveler::LightType;
using shoveler::Material;
using shoveler::MaterialType;
using shoveler::Model;
using shoveler::PolygonMode;
using improbable::EntityAcl;
using improbable::EntityAclData;
using improbable::Metadata;
using improbable::Persistence;
using improbable::Position;
using improbable::WorkerAttributeSet;
using improbable::WorkerRequirementSet;

static const float PI = 3.14159265358979323846f;

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <seed snapshot file>" << std::endl;
		return 1;
	}

	std::string path(argv[1]);

	const worker::ComponentRegistry& components = worker::Components<
		shoveler::Client,
		shoveler::Light,
		shoveler::Model,
		improbable::EntityAcl,
		improbable::Metadata,
		improbable::Persistence,
		improbable::Position>{};

	Color grayColor{0.7f, 0.7f, 0.7f};
	Color whiteColor{1.0f, 1.0f, 1.0f};
	Drawable cubeDrawable{DrawableType::CUBE};
	Drawable quadDrawable{DrawableType::QUAD};
	Drawable pointDrawable{DrawableType::POINT};
	Material grayColorMaterial{MaterialType::COLOR, grayColor, {}};
	Material whiteParticleMaterial{MaterialType::PARTICLE, whiteColor, {}};
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
	planeEntity.Add<Position>({{0, -2, 0}});
	planeEntity.Add<Model>({quadDrawable, grayColorMaterial, {PI / 2.0f, 0.0f, 0.0f}, {25.0f, 25.0f, 1.0f}, true, false, false, true, PolygonMode::FILL});
	EntityAclData planeEntityAclData(clientRequirementSet, emptyComponentAclMap);
	planeEntity.Add<EntityAcl>({clientRequirementSet, emptyComponentAclMap});
	entities[2] = planeEntity;

	worker::Entity cubeEntity;
	cubeEntity.Add<Metadata>({"cube"});
	cubeEntity.Add<Persistence>({});
	cubeEntity.Add<Position>({{0, 0, 5}});
	cubeEntity.Add<Model>({cubeDrawable, grayColorMaterial, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, true, false, false, true, PolygonMode::FILL});
	cubeEntity.Add<EntityAcl>({clientRequirementSet, emptyComponentAclMap});
	entities[3] = cubeEntity;

	worker::Entity lightEntity;
	lightEntity.Add<Metadata>({"light"});
	lightEntity.Add<Persistence>({});
	lightEntity.Add<Position>({{-1, 5, -1}});
	lightEntity.Add<Model>({pointDrawable, whiteParticleMaterial, {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f}, true, true, false, false, PolygonMode::FILL});
	lightEntity.Add<Light>({LightType::POINT, 1024, 1024, 1, 0.01f, 80.0f, {1.0f, 1.0f, 1.0f}, {}});
	lightEntity.Add<EntityAcl>({clientRequirementSet, emptyComponentAclMap});
	entities[4] = lightEntity;

	worker::SaveSnapshot(components, path, entities);
	return 0;
}
