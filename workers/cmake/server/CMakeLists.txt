include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_SERVER_SRC
	server.cpp
)

add_executable(ShovelerServer ${SHOVELER_SERVER_SRC})
target_link_libraries(ShovelerServer schema worker_sdk shoveler_spatialos_common shoveler_spatialos_worker_view_base)
