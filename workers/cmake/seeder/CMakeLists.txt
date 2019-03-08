include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SEEDER_LIGHTS_SRC
	lights.cpp
)

set(SEEDER_TILES_SRC
	tiles.cpp
	tiles/map.cpp
	tiles/map.h
	tiles/tileset.cpp
	tiles/tileset.h
)

add_executable(seeder_lights ${SEEDER_LIGHTS_SRC})
target_link_libraries(seeder_lights schema worker_sdk shoveler_base)

add_executable(seeder_tiles ${SEEDER_TILES_SRC})
target_link_libraries(seeder_tiles schema worker_sdk shoveler_base)

add_custom_command(TARGET seeder_tiles POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:seeder_tiles>/assets/tilesets
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_tilesets_autumn} $<TARGET_FILE_DIR:seeder_tiles>/assets/tilesets/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_tilesets_spring} $<TARGET_FILE_DIR:seeder_tiles>/assets/tilesets/
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:seeder_tiles>/assets/characters
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player1} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player2} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player3} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player4} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
)

add_dependencies(seeder_tiles shoveler_spatialos_assets)
