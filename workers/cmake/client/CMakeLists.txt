include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_CLIENT_SRC
	canvas.cpp
	canvas.h
	chunk.cpp
	chunk.h
	client.cpp
	configuration.cpp
	configuration.h
	connect.cpp
	connect.h
	coordinate_mapping.h
	drawable.cpp
	drawable.h
	interest.cpp
	interest.h
	light.cpp
	light.h
	material.cpp
	material.h
	metadata.cpp
	metadata.h
	model.cpp
	model.h
	position.cpp
	position.h
	resource.cpp
	resource.h
	texture.cpp
	texture.h
	tile_sprite.cpp
	tile_sprite.h
	tile_sprite_animation.cpp
	tile_sprite_animation.h
	tilemap.cpp
	tilemap.h
	tilemap_tiles.cpp
	tilemap_tiles.h
	tileset.cpp
	tileset.h)

add_executable(ShovelerClient ${SHOVELER_CLIENT_SRC})
target_link_libraries(ShovelerClient schema worker_sdk shoveler_opengl)
