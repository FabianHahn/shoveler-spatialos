#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include <shoveler.h>
#include <improbable/worker.h>
#include <shoveler/view.h>

extern "C" {
#include <glib.h>

#include <shoveler/view/canvas.h>
#include <shoveler/view/client.h>
#include <shoveler/view/chunk.h>
#include <shoveler/view/chunk_layer.h>
#include <shoveler/view/drawable.h>
#include <shoveler/view/image.h>
#include <shoveler/view/material.h>
#include <shoveler/view/model.h>
#include <shoveler/view/position.h>
#include <shoveler/view/resource.h>
#include <shoveler/view/sampler.h>
#include <shoveler/view/texture.h>
#include <shoveler/view/tile_sprite.h>
#include <shoveler/view/tile_sprite_animation.h>
#include <shoveler/view/tilemap_colliders.h>
#include <shoveler/view/tilemap_tiles.h>
#include <shoveler/view/tilemap.h>
#include <shoveler/view/tileset.h>
#include <shoveler/log.h>
};

#include "interest.h"

using improbable::EntityAcl;
using improbable::ComponentInterest;
using improbable::Metadata;
using improbable::Position;
using shoveler::Canvas;
using shoveler::Chunk;
using shoveler::ChunkLayer;
using shoveler::Client;
using shoveler::Drawable;
using shoveler::Image;
using shoveler::Material;
using shoveler::Model;
using shoveler::Resource;
using shoveler::Sampler;
using shoveler::Texture;
using shoveler::TileSprite;
using shoveler::TileSpriteAnimation;
using shoveler::Tilemap;
using shoveler::TilemapColliders;
using shoveler::TilemapTiles;
using shoveler::Tileset;
using std::endl;
using std::map;
using std::ofstream;
using std::unordered_map;
using std::set;
using std::string;
using worker::ComponentId;
using worker::EntityId;
using worker::List;

using Query = ComponentInterest::Query;
using QueryConstraint = ComponentInterest::QueryConstraint;

static const unordered_map<string, ComponentId> COMPONENT_IDS = {
	{shovelerComponentTypeIdCanvas,		    		Canvas::ComponentId},
	{shovelerComponentTypeIdChunk,		    		Chunk::ComponentId},
    {shovelerComponentTypeIdChunkLayer,				ChunkLayer::ComponentId},
	{shovelerComponentTypeIdClient,				    Client::ComponentId},
	{shovelerComponentTypeIdDrawable,				Drawable::ComponentId},
	{shovelerComponentTypeIdImage,					Image::ComponentId},
	{shovelerComponentTypeIdMaterial,				Material::ComponentId},
	{shovelerComponentTypeIdModel,				    Model::ComponentId},
	{shovelerComponentTypeIdPosition,				Position::ComponentId},
	{shovelerComponentTypeIdResource,				Resource::ComponentId},
	{shovelerComponentTypeIdTexture,				Texture::ComponentId},
    {shovelerComponentTypeIdSampler,			    Sampler::ComponentId},
	{shovelerComponentTypeIdTileSprite,			    TileSprite::ComponentId},
	{shovelerComponentTypeIdTileSpriteAnimation,	TileSpriteAnimation::ComponentId},
	{shovelerComponentTypeIdTilemap,				Tilemap::ComponentId},
    {shovelerComponentTypeIdTilemapColliders,		TilemapColliders::ComponentId},
	{shovelerComponentTypeIdTilemapTiles,			TilemapTiles::ComponentId},
	{shovelerComponentTypeIdTileset,				Tileset::ComponentId},
};

ComponentInterest computeViewInterest(ShovelerView *view, bool useAbsoluteConstraint, ShovelerVector3 absolutePosition, double edgeLength)
{
	map<EntityId, set<ComponentId>> dependencies;

	GHashTableIter iter;
	ShovelerViewQualifiedComponent *dependencyTarget;
	GQueue *dependencySourceList;
	g_hash_table_iter_init(&iter, view->reverseDependencies);
	while(g_hash_table_iter_next(&iter, (gpointer *) &dependencyTarget, (gpointer *) &dependencySourceList)) {
		unordered_map<string, ComponentId>::const_iterator componentIdQuery = COMPONENT_IDS.find(
			dependencyTarget->componentTypeId);
		if (componentIdQuery == COMPONENT_IDS.end()) {
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

		dependencies[dependencyTarget->entityId].insert(componentIdQuery->second);
	}

	List<Query> queries;
	for(map<EntityId, set<ComponentId>>::const_iterator iter = dependencies.begin(); iter != dependencies.end(); ++iter) {
		QueryConstraint constraint;
		constraint.set_entity_id_constraint({iter->first});

		List<ComponentId> dependencyComponentIds(iter->second.begin(), iter->second.end());
		dependencyComponentIds.emplace_back(EntityAcl::ComponentId);
		dependencyComponentIds.emplace_back(Metadata::ComponentId);

		Query query;
		query.set_constraint(constraint);
		query.set_result_component_id(dependencyComponentIds);

		queries.emplace_back(query);
	}

	if(useAbsoluteConstraint) {
		QueryConstraint absoluteConstraint;
		absoluteConstraint.set_box_constraint({{{absolutePosition.values[0], absolutePosition.values[1], absolutePosition.values[2]}, {edgeLength, 9999, edgeLength}}});
		Query absoluteQuery;
		absoluteQuery.set_constraint(absoluteConstraint);
		absoluteQuery.set_full_snapshot_result({true});
		queries.emplace_back(absoluteQuery);
	} else {
		QueryConstraint relativeConstraint;
		relativeConstraint.set_relative_box_constraint({{{edgeLength, 9999, edgeLength}}});
		Query relativeQuery;
		relativeQuery.set_constraint(relativeConstraint);
		relativeQuery.set_full_snapshot_result({true});
		queries.emplace_back(relativeQuery);
	}

	ComponentInterest interest;
	interest.set_queries(queries);
	return interest;
}
