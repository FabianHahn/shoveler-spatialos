include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_CLIENT_SRC
	client.cpp
)

add_executable(ShovelerClient ${SHOVELER_CLIENT_SRC})
target_link_libraries(ShovelerClient schema worker_sdk shoveler_opengl)
