include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_UPDATER_SRC
	updater.c
)

add_executable(ShovelerUpdater ${SHOVELER_UPDATER_SRC})
target_link_libraries(ShovelerUpdater PNG::PNG ZLIB::ZLIB shoveler_worker_common worker_sdk::c_worker_sdk)

if(WIN32)
	add_custom_command(
		TARGET ShovelerUpdater
		POST_BUILD
		COMMENT "Creating updater release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "updater-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerUpdater>"
		WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerUpdater>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
			TARGET ShovelerUpdater
			POST_BUILD
			COMMENT "Creating updater release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "updater-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerUpdater>"
	)
endif()
