include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_CLIENT_SRC
	client.c
	configuration.c
	configuration.h
	interest.c
	interest.h
	spatialos_client_schema.c
	spatialos_client_schema.h
)

add_executable(ShovelerClient ${SHOVELER_CLIENT_SRC})
target_link_libraries(ShovelerClient shoveler_client shoveler_opengl PNG::PNG ZLIB::ZLIB shoveler_worker_common worker_sdk::c_worker_sdk)

add_custom_command(
	TARGET ShovelerClient
	POST_BUILD
	COMMENT "Copying ShovelerClient binary to assembly directory."
	COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:ShovelerClient>/assembly
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ShovelerClient> $<TARGET_FILE_DIR:ShovelerClient>/assembly/$<TARGET_FILE_NAME:ShovelerClient>
)

if(WIN32)
	add_custom_command(
		TARGET ShovelerClient
		POST_BUILD
		COMMENT "Creating client release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "client-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerClient>"
		WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerClient>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
			TARGET ShovelerClient
			POST_BUILD
			COMMENT "Creating client release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "client-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerClient>"
	)
endif()
