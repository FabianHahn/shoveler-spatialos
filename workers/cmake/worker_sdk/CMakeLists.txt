if(MSVC)
  set(WORKER_SDK_DIR_RELEASE "${CMAKE_SOURCE_DIR}/worker_sdk/windows/release")
  set(WORKER_SDK_DIR_DEBUG "${CMAKE_SOURCE_DIR}/worker_sdk/windows/debug")

  # Find all the worker SDK libraries.
  find_package(Threads)
  find_library(LIB_CORE_SDK_RELEASE CoreSdk "${WORKER_SDK_DIR_RELEASE}/lib" DEFAULT_PATH)
  find_library(LIB_CORE_SDK_DEBUG CoreSdk "${WORKER_SDK_DIR_DEBUG}/lib" DEFAULT_PATH)
  find_library(LIB_WORKER_SDK_RELEASE WorkerSdk "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_WORKER_SDK_DEBUG WorkerSdk "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)
  find_library(LIB_PROTOBUF_RELEASE NAMES protobuf libprotobuf PATHS "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_PROTOBUF_DEBUG NAMES protobuf libprotobuf PATHS "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)
  find_library(LIB_RAKNET_RELEASE RakNetLibStatic "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_RAKNET_DEBUG RakNetLibStatic "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)
  find_library(LIB_SSL_RELEASE ssl "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_SSL_DEBUG ssl "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)
  find_library(LIB_ZLIB_RELEASE NAMES z zlibstatic PATHS "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_ZLIB_DEBUG NAMES z zlibstatic PATHS "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)
  find_library(LIB_GPR_RELEASE NAMES gpr PATHS "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_GPR_DEBUG NAMES gpr PATHS "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)
  find_library(LIB_GRPC_RELEASE NAMES grpc PATHS "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_GRPC_DEBUG NAMES grpc PATHS "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)
  find_library(LIB_GRPCXX_RELEASE NAMES grpc++ PATHS "${WORKER_SDK_DIR_RELEASE}/lib" NO_DEFAULT_PATH)
  find_library(LIB_GRPCXX_DEBUG NAMES grpc++ PATHS "${WORKER_SDK_DIR_DEBUG}/lib" NO_DEFAULT_PATH)

  # Worker SDK target.
  add_library(worker_sdk INTERFACE)
  target_include_directories(worker_sdk SYSTEM INTERFACE
    optimized "${WORKER_SDK_DIR_RELEASE}/include" debug "${WORKER_SDK_DIR_DEBUG}/include")
  target_link_libraries(worker_sdk INTERFACE
    optimized ${LIB_WORKER_SDK_RELEASE} debug ${LIB_WORKER_SDK_DEBUG}
    optimized ${LIB_CORE_SDK_RELEASE} debug ${LIB_CORE_SDK_DEBUG}
    ${CMAKE_THREAD_LIBS_INIT}
    optimized ${LIB_PROTOBUF_RELEASE} debug ${LIB_PROTOBUF_DEBUG}
    optimized ${LIB_RAKNET_RELEASE} debug ${LIB_RAKNET_DEBUG}
    optimized ${LIB_ZLIB_RELEASE} debug ${LIB_ZLIB_DEBUG}
    optimized ${LIB_GRPCXX_RELEASE} debug ${LIB_GRPCXX_DEBUG}
    optimized ${LIB_GRPC_RELEASE} debug ${LIB_GRPC_DEBUG}
    optimized ${LIB_GPR_RELEASE} debug ${LIB_GPR_DEBUG}
    optimized ${LIB_SSL_RELEASE} debug ${LIB_SSL_DEBUG}
    ${CMAKE_DL_LIBS})

else()
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
endif()
