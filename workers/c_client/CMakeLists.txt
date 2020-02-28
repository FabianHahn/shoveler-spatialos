include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_C_CLIENT_SRC
	client.c
	configuration.c
	configuration.h
	connect.c
	connect.h
	interest.c
	interest.h
	schema.c
	schema.h
)

add_executable(ShovelerCClient ${SHOVELER_C_CLIENT_SRC})
target_link_libraries(ShovelerCClient shoveler_opengl worker_sdk::c_worker_sdk)

add_custom_command(
	TARGET ShovelerCClient
	POST_BUILD
	COMMENT "Copying ShovelerCClient binary to assembly directory."
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ShovelerCClient>/assembly
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ShovelerCClient> $<TARGET_FILE_DIR:ShovelerCClient>/assembly/$<TARGET_FILE_NAME:ShovelerCClient>
)

if(WIN32)
	add_custom_command(
		TARGET ShovelerCClient
		POST_BUILD
		COMMENT "Creating C client release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "c_client-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerCClient>"
		WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerCClient>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
			TARGET ShovelerCClient
			POST_BUILD
			COMMENT "Creating C client release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "c_client-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerCClient>"
	)
endif()
