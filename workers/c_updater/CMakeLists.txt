include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_C_UPDATER_SRC
	updater.c
)

add_executable(ShovelerCUpdater ${SHOVELER_C_UPDATER_SRC})
target_link_libraries(ShovelerCUpdater PNG::PNG ZLIB::ZLIB shoveler_worker_common worker_sdk::c_worker_sdk)

if(WIN32)
	add_custom_command(
		TARGET ShovelerCUpdater
		POST_BUILD
		COMMENT "Creating C updater release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "c_updater-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerCUpdater>"
		WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerCUpdater>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
			TARGET ShovelerCUpdater
			POST_BUILD
			COMMENT "Creating C updater release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "c_updater-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerCUpdater>"
	)
endif()
