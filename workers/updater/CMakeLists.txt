include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_UPDATER_SRC
	updater.cpp
	../bot_client/connect.cpp
	../bot_client/connect.h
)

add_executable(ShovelerAssetUpdater ${SHOVELER_UPDATER_SRC})
target_link_libraries(ShovelerAssetUpdater shoveler_base schema worker_sdk::cpp_worker_sdk)

if(WIN32)
	add_custom_command(
		TARGET ShovelerAssetUpdater
		POST_BUILD
		COMMENT "Creating updater release archive."
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "updater-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerAssetUpdater>"
		WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerAssetUpdater>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
			TARGET ShovelerAssetUpdater
			POST_BUILD
			COMMENT "Creating updater release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "updater-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerAssetUpdater>"
	)
endif()
