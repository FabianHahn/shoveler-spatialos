include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_BOT_CLIENT_SRC
	bot_client.cpp
	connect.cpp
	connect.h
)

add_executable(ShovelerBotClient ${SHOVELER_BOT_CLIENT_SRC})
target_link_libraries(ShovelerBotClient shoveler_base schema worker_sdk::cpp_worker_sdk)

if(WIN32)
	add_custom_command(
			TARGET ShovelerBotClient
			POST_BUILD
			COMMENT "Creating bot client release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "bot_client-x86_64-win32.zip" --format=zip "$<TARGET_FILE_NAME:ShovelerBotClient>"
			WORKING_DIRECTORY "$<TARGET_FILE_DIR:ShovelerBotClient>"
	)
elseif(UNIX AND NOT APPLE)
	add_custom_command(
			TARGET ShovelerBotClient
			POST_BUILD
			COMMENT "Creating bot client release archive."
			COMMAND ${CMAKE_COMMAND} -E tar "cfvz" "bot_client-x86_64-linux.tar.gz" "$<TARGET_FILE:ShovelerBotClient>"
	)
endif()
