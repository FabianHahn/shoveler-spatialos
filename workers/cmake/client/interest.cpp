#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include <shoveler.h>
#include <improbable/worker.h>
#include <shoveler/view.h>

extern "C" {
#include <shoveler/view/canvas.h>
#include <shoveler/view/client.h>
#include <shoveler/view/chunk.h>
#include <shoveler/view/drawable.h>
#include <shoveler/view/material.h>
#include <shoveler/view/model.h>
#include <shoveler/view/position.h>
#include <shoveler/view/resources.h>
#include <shoveler/view/texture.h>
#include <shoveler/view/tile_sprite.h>
#include <shoveler/view/tile_sprite_animation.h>
#include <shoveler/view/tilemap_tiles.h>
#include <shoveler/view/tilemap.h>
#include <shoveler/view/tileset.h>
#include <shoveler/log.h>
};

#include "interest.h"

using improbable::ComponentInterest;
using improbable::Position;
using shoveler::Canvas;
using shoveler::Chunk;
using shoveler::Client;
using shoveler::Drawable;
using shoveler::Material;
using shoveler::Model;
using shoveler::Resource;
using shoveler::Texture;
using shoveler::TileSprite;
using shoveler::TileSpriteAnimation;
using shoveler::Tilemap;
using shoveler::TilemapTiles;
using shoveler::Tileset;
using std::endl;
using std::ofstream;
using std::unordered_map;
using std::string;
using worker::ComponentId;
using worker::List;

using Query = ComponentInterest::Query;
using QueryConstraint = ComponentInterest::QueryConstraint;

static const unordered_map<string, ComponentId> COMPONENT_IDS = {
	{shovelerViewCanvasComponentName,				Canvas::ComponentId},
	{shovelerViewChunkComponentName,				Chunk::ComponentId},
	{shovelerViewClientComponentName,				Client::ComponentId},
	{shovelerViewDrawableComponentName,				Drawable::ComponentId},
	{shovelerViewMaterialComponentName,				Material::ComponentId},
	{shovelerViewModelComponentName,				Model::ComponentId},
	{shovelerViewPositionComponentName,				Position::ComponentId},
	{shovelerViewResourceComponentName,				Resource::ComponentId},
	{shovelerViewTextureComponentName,				Texture::ComponentId},
	{shovelerViewTileSpriteComponentName,			TileSprite::ComponentId},
	{shovelerViewTileSpriteAnimationComponentName,	TileSpriteAnimation::ComponentId},
	{shovelerViewTilemapComponentName,				Tilemap::ComponentId},
	{shovelerViewTilemapTilesComponentName,			TilemapTiles::ComponentId},
	{shovelerViewTilesetComponentName,				Tileset::ComponentId},
};

ComponentInterest computeViewInterest(ShovelerView *view)
{
	List<Query> queries;

	GHashTableIter iter;
	ShovelerViewQualifiedComponent *dependencyTarget;
	GQueue *dependencySourceList;
	g_hash_table_iter_init(&iter, view->reverseDependencies);
	while(g_hash_table_iter_next(&iter, (gpointer *) &dependencyTarget, (gpointer *) &dependencySourceList)) {
		QueryConstraint constraint;
		constraint.set_entity_id_constraint({dependencyTarget->entityId});

		Query query;
		query.set_constraint(constraint);

		unordered_map<string, ComponentId>::const_iterator componentIdQuery = COMPONENT_IDS.find(dependencyTarget->componentName);
		if(componentIdQuery != COMPONENT_IDS.end()) {
			query.set_result_component_id({componentIdQuery->second});
		} else {
			for(GList *dependencySourceListIter = dependencySourceList->head; dependencySourceListIter != NULL; dependencySourceListIter = dependencySourceListIter->next) {
				const ShovelerViewQualifiedComponent *dependencySource = (ShovelerViewQualifiedComponent *) dependencySourceListIter->data;
				shovelerLogWarning(
					"Component '%s' of entity %lld declared a dependency on component '%s' of entity %lld, but the component ID map doesn't contain an entry for this target."
						"Setting component interest query result type to full snapshot, which might be inefficient.",
					dependencySource->componentName,
					dependencySource->entityId,
					dependencyTarget->componentName,
					dependencyTarget->entityId);
			}
			query.set_full_snapshot_result({true});
		}

		queries.emplace_back(query);
	}

	ComponentInterest interest;
	interest.set_queries(queries);
	return interest;
}
