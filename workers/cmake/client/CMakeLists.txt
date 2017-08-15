include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_CLIENT_SRC
	main.cpp
	worker_view.c
)

add_executable(ShovelerClient ${SHOVELER_CLIENT_SRC})
target_link_libraries(ShovelerClient schema worker_sdk shoveler)
