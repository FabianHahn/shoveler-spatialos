include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_C_SERVER_SRC
	configuration.c
	configuration.h
	server.c
)

add_executable(ShovelerCServer ${SHOVELER_C_SERVER_SRC})
target_link_libraries(ShovelerCServer shoveler_worker_common worker_sdk::c_worker_sdk)

add_custom_command(
	TARGET ShovelerCServer
	POST_BUILD
	COMMENT "Copying ShovelerCServer binary to assembly directory."
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ShovelerCServer>/assembly
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ShovelerCServer> $<TARGET_FILE_DIR:ShovelerCServer>/assembly/$<TARGET_FILE_NAME:ShovelerCServer>
)
