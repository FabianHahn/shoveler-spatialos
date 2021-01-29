include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SEEDER_LIGHTS_SRC
	lights.c
)

set(SEEDER_TILES_SRC
	tiles.cpp
	tiles/map.cpp
	tiles/map.h
	tiles/tileset.cpp
	tiles/tileset.h
)

add_executable(seeder_lights ${SEEDER_LIGHTS_SRC})
target_link_libraries(seeder_lights shoveler_worker_common worker_sdk::c_worker_sdk shoveler_base)

add_custom_command(TARGET seeder_lights POST_BUILD
	COMMAND seeder_lights ${CMAKE_CURRENT_BINARY_DIR}/lights.snapshot
)

add_executable(seeder_tiles ${SEEDER_TILES_SRC})
target_link_libraries(seeder_tiles schema worker_sdk::cpp_worker_sdk shoveler_base)

add_custom_command(TARGET seeder_tiles POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:seeder_tiles>/assets/tilesets
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_tilesets_autumn} $<TARGET_FILE_DIR:seeder_tiles>/assets/tilesets/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_tilesets_spring} $<TARGET_FILE_DIR:seeder_tiles>/assets/tilesets/
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:seeder_tiles>/assets/characters
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player1} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player2} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player3} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player4} $<TARGET_FILE_DIR:seeder_tiles>/assets/characters/
	COMMAND seeder_tiles
		$<TARGET_FILE_DIR:seeder_tiles>/assets/tilesets/spring.png 7 2
		$<TARGET_FILE_DIR:seeder_tiles>/assets/characters/player1.png
		$<TARGET_FILE_DIR:seeder_tiles>/assets/characters/player2.png
		$<TARGET_FILE_DIR:seeder_tiles>/assets/characters/player3.png
		$<TARGET_FILE_DIR:seeder_tiles>/assets/characters/player4.png
		5
		${CMAKE_CURRENT_BINARY_DIR}/tiles.snapshot
)

add_dependencies(seeder_tiles shoveler_spatialos_assets)
