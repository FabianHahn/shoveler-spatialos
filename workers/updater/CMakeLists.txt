include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_UPDATER_SRC
	updater.cpp
	../client/connect.cpp
	../client/connect.h
)

add_executable(ShovelerAssetUpdater ${SHOVELER_UPDATER_SRC})
target_link_libraries(ShovelerAssetUpdater schema worker_sdk shoveler_base)
