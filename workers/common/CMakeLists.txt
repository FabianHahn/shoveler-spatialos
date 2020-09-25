include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_WORKER_COMMON_SRC
	include/shoveler/connect.h
	include/shoveler/schema.h
	include/shoveler/worker_log.h
	src/connect.c
	src/schema.c
	src/worker_log.c
)

add_library(shoveler_worker_common ${SHOVELER_WORKER_COMMON_SRC})
add_library(shoveler::shoveler_worker_common ALIAS shoveler_worker_common)
set_property(TARGET shoveler_worker_common PROPERTY C_STANDARD 11)

target_include_directories(shoveler_worker_common
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PUBLIC $<INSTALL_INTERFACE:include>
		PRIVATE src)

target_link_libraries(shoveler_worker_common PUBLIC shoveler::shoveler_base worker_sdk::c_worker_sdk)
