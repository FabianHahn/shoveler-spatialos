cmake_minimum_required(VERSION 3.1)
project(shoveler-spatialos)

set(CMAKE_CXX_STANDARD 11)

set(SHOVELER_BUILD_SAMPLE OFF CACHE BOOL "Disable building shoveler sample")

add_subdirectory(client)
add_subdirectory(shoveler_lib)
add_subdirectory(worker_sdk)

