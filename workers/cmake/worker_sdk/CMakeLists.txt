set(WORKER_SDK_DIR "${CMAKE_SOURCE_DIR}/worker_sdk/linux")

# Find all the worker SDK libraries.
find_package(Threads)
find_library(LIB_CORE_SDK CoreSdk "${WORKER_SDK_DIR}/lib" DEFAULT_PATH)
find_library(LIB_WORKER_SDK WorkerSdk "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_PROTOBUF NAMES protobuf libprotobuf PATHS "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_RAKNET RakNetLibStatic "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_SSL ssl "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_ZLIB NAMES z zlibstatic PATHS "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_GPR NAMES gpr PATHS "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_GRPC NAMES grpc PATHS "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)
find_library(LIB_GRPCXX NAMES grpc++ PATHS "${WORKER_SDK_DIR}/lib" NO_DEFAULT_PATH)

# Worker SDK target.
add_library(worker_sdk INTERFACE)
target_include_directories(worker_sdk SYSTEM INTERFACE "${WORKER_SDK_DIR}/include")
target_link_libraries(worker_sdk INTERFACE
  ${LIB_WORKER_SDK} ${LIB_CORE_SDK} ${CMAKE_THREAD_LIBS_INIT} ${LIB_PROTOBUF}
  ${LIB_RAKNET} ${LIB_ZLIB} ${LIB_GRPCXX} ${LIB_GRPC} ${LIB_GPR} ${LIB_SSL} ${CMAKE_DL_LIBS})
