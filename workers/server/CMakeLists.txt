include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_SERVER_SRC
	server.cpp
)

add_executable(ShovelerServer ${SHOVELER_SERVER_SRC})
target_link_libraries(ShovelerServer schema worker_sdk shoveler_base)

add_custom_command(
	TARGET ShovelerServer
	POST_BUILD
	COMMENT "Copying ShovelerServer binary to assembly directory."
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ShovelerServer>/assembly
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ShovelerServer> $<TARGET_FILE_DIR:ShovelerServer>/assembly/$<TARGET_FILE_NAME:ShovelerServer>
)

if(WIN32)
	add_custom_command(
		TARGET ShovelerServer
		POST_BUILD
		COMMENT "Creating server release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "server-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerServer>"
		WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerServer>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
		TARGET ShovelerServer
		POST_BUILD
		COMMENT "Creating server release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "server-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerServer>"
	)
endif()
