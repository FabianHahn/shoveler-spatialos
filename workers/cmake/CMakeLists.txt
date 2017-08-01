cmake_minimum_required(VERSION 3.1)
project(shoveler-spatialos)

set(CMAKE_CXX_STANDARD 11)

set(SHOVELER_BUILD_SAMPLE OFF CACHE BOOL "Disable building shoveler sample")

set(GENERATED_CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
set(SCHEMA_SRC
	${GENERATED_CODE_DIR}/shoveler.cc
	${GENERATED_CODE_DIR}/improbable/standard_library.cc
)
add_library(schema STATIC ${SCHEMA_SRC})
target_include_directories(schema SYSTEM PUBLIC ${GENERATED_CODE_DIR})
target_link_libraries(schema PRIVATE worker_sdk)

add_subdirectory(client)
add_subdirectory(shoveler_lib)
add_subdirectory(worker_sdk)

