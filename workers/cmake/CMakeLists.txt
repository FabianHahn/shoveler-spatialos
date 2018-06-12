cmake_minimum_required(VERSION 3.1)
project(shoveler-spatialos)

set(CMAKE_CXX_STANDARD 11)

set(SHOVELER_BUILD_TESTS OFF CACHE BOOL "Disable building shoveler tests")
set(SHOVELER_BUILD_EXAMPLES OFF CACHE BOOL "Disable building shoveler examples")

set(GENERATED_CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
set(SCHEMA_SRC
	${GENERATED_CODE_DIR}/shoveler.cc
	${GENERATED_CODE_DIR}/improbable/standard_library.cc
)
add_library(schema STATIC ${SCHEMA_SRC})
target_include_directories(schema SYSTEM PUBLIC ${GENERATED_CODE_DIR})
target_link_libraries(schema PRIVATE worker_sdk)

add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(seeder)
add_subdirectory(server)
add_subdirectory(shoveler)
add_subdirectory(view)
add_subdirectory(worker_sdk)

