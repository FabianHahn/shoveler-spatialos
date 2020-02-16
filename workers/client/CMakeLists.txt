include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_CLIENT_SRC
	canvas.cpp
	canvas.h
	chunk.cpp
	chunk.h
	chunk_layer.cpp
	chunk_layer.h
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
	sampler.cpp
	sampler.h
	sprite.cpp
	sprite.h
	texture.cpp
	texture.h
	tile_sprite.cpp
	tile_sprite.h
	tile_sprite_animation.cpp
	tile_sprite_animation.h
	tilemap.cpp
	tilemap.h
	tilemap_colliders.cpp
	tilemap_colliders.h
	tilemap_tiles.cpp
	tilemap_tiles.h
	tileset.cpp
	tileset.h)

add_executable(ShovelerClient ${SHOVELER_CLIENT_SRC})
target_link_libraries(ShovelerClient shoveler_opengl schema worker_sdk::cpp_worker_sdk)

add_custom_command(
	TARGET ShovelerClient
	POST_BUILD
	COMMENT "Copying ShovelerClient binary to assembly directory."
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ShovelerClient>/assembly
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ShovelerClient> $<TARGET_FILE_DIR:ShovelerClient>/assembly/$<TARGET_FILE_NAME:ShovelerClient>
)

if(WIN32)
	add_custom_command(
		TARGET ShovelerClient
		POST_BUILD
		COMMENT "Creating client release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "client-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerClient>"
		WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerClient>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
			TARGET ShovelerClient
			POST_BUILD
			COMMENT "Creating client release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "client-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerClient>"
	)
endif()
