include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SEEDER_SRC
	seeder.cpp
)

add_executable(seeder ${SEEDER_SRC})
target_link_libraries(seeder schema worker_sdk shoveler_base)

add_custom_command(TARGET seeder POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:seeder>/assets/tilesets
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_tilesets_autumn} $<TARGET_FILE_DIR:seeder>/assets/tilesets/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_tilesets_spring} $<TARGET_FILE_DIR:seeder>/assets/tilesets/
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:seeder>/assets/characters
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player1} $<TARGET_FILE_DIR:seeder>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player2} $<TARGET_FILE_DIR:seeder>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player3} $<TARGET_FILE_DIR:seeder>/assets/characters/
	COMMAND ${CMAKE_COMMAND} -E copy ${shoveler_spatialos_assets_characters_player4} $<TARGET_FILE_DIR:seeder>/assets/characters/
)

add_dependencies(seeder shoveler_spatialos_assets)
