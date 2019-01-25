include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SEEDER_SRC
	seeder.cpp
)

add_executable(seeder ${SEEDER_SRC})
target_link_libraries(seeder schema worker_sdk shoveler_base)
