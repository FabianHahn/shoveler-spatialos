cmake_minimum_required(VERSION 3.16)

project(shoveler-spatialos)

set(SPATIALOS_SDK_VERSION "16.1.0")

# download external dependencies at configuration time
configure_file(CMakeLists.txt.external.in external/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external"
)
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/external"
)

if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

set(CMAKE_CXX_STANDARD 11)

set(SHOVELER_BUILD_TESTS OFF CACHE BOOL "Disable building shoveler tests")
set(SHOVELER_BUILD_EXAMPLES OFF CACHE BOOL "Disable building shoveler examples")

add_subdirectory(assets)
add_subdirectory(schema)
add_subdirectory(seeders)
add_subdirectory(shoveler)
add_subdirectory(thirdparty)
add_subdirectory(workers)
