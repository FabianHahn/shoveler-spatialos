include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${GLIB_INCLUDE_DIRS} ${GLAD_INCLUDE_DIRS})

set(SHOVELER_CLIENT_SRC
	main.cpp
)

add_executable(ShovelerClient ${SHOVELER_CLIENT_SRC})
target_link_libraries(ShovelerClient worker_sdk)
