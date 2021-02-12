set(WORKER_SDK_C_HEADERS "${CMAKE_BINARY_DIR}/external/worker_sdk/c_headers/include")

if(MSVC)
	set(WORKER_SDK_DIR_RELEASE "${CMAKE_BINARY_DIR}/external/worker_sdk/windows/release")
	set(WORKER_SDK_DIR_DEBUG "${CMAKE_BINARY_DIR}/external/worker_sdk/windows/debug")

	# Find all the worker SDK libraries.
	find_package(Threads)
	find_library(LIB_IMPROBABLE_WORKER_RELEASE improbable_worker PATHS "${WORKER_SDK_DIR_RELEASE}" NO_DEFAULT_PATH REQUIRED)
	find_library(LIB_IMPROBABLE_WORKER_DEBUG improbable_worker PATHS "${WORKER_SDK_DIR_DEBUG}" NO_DEFAULT_PATH REQUIRED)
	find_library(LIB_RAKNET_RELEASE RakNetLibStatic PATHS "${WORKER_SDK_DIR_RELEASE}" NO_DEFAULT_PATH REQUIRED)
	find_library(LIB_RAKNET_DEBUG RakNetLibStatic PATHS "${WORKER_SDK_DIR_DEBUG}" NO_DEFAULT_PATH REQUIRED)
	find_library(LIB_SSL_RELEASE ssl PATHS "${WORKER_SDK_DIR_RELEASE}" NO_DEFAULT_PATH REQUIRED)
	find_library(LIB_SSL_DEBUG ssl PATHS "${WORKER_SDK_DIR_DEBUG}" NO_DEFAULT_PATH REQUIRED)

	# Worker SDK targets.
	add_library(worker_sdk::RakNetLibStatic STATIC IMPORTED)
	set_target_properties(worker_sdk::RakNetLibStatic PROPERTIES
		IMPORTED_LOCATION_DEBUG ${LIB_RAKNET_DEBUG}
		IMPORTED_LOCATION_RELEASE ${LIB_RAKNET_RELEASE})

	add_library(worker_sdk::ssl STATIC IMPORTED)
	set_target_properties(worker_sdk::ssl PROPERTIES
		IMPORTED_LOCATION_DEBUG ${LIB_SSL_DEBUG}
		IMPORTED_LOCATION_RELEASE ${LIB_SSL_RELEASE})

	add_library(worker_sdk::c_worker_sdk STATIC IMPORTED GLOBAL)
	set_target_properties(worker_sdk::c_worker_sdk PROPERTIES
			IMPORTED_LOCATION_DEBUG ${LIB_IMPROBABLE_WORKER_DEBUG}
			IMPORTED_LOCATION_RELEASE ${LIB_IMPROBABLE_WORKER_RELEASE}
			INTERFACE_INCLUDE_DIRECTORIES "${WORKER_SDK_C_HEADERS}")
	target_link_libraries(worker_sdk::c_worker_sdk INTERFACE worker_sdk::RakNetLibStatic worker_sdk::ssl zlibstatic)
else()
	set(WORKER_SDK_DIR "${CMAKE_BINARY_DIR}/external/worker_sdk/linux")

	# Find all the worker SDK libraries.
	find_package(Threads)
	find_library(LIB_IMPROBABLE_WORKER improbable_worker PATHS "${WORKER_SDK_DIR}" NO_DEFAULT_PATH REQUIRED)
	find_library(LIB_RAKNET RakNetLibStatic PATHS "${WORKER_SDK_DIR}" NO_DEFAULT_PATH REQUIRED)
	find_library(LIB_SSL ssl PATHS "${WORKER_SDK_DIR}" NO_DEFAULT_PATH REQUIRED)

	# Worker SDK targets.
	add_library(worker_sdk::RakNetLibStatic STATIC IMPORTED)
	set_target_properties(worker_sdk::RakNetLibStatic PROPERTIES
		IMPORTED_LOCATION ${LIB_RAKNET})

	add_library(worker_sdk::ssl STATIC IMPORTED)
	set_target_properties(worker_sdk::ssl PROPERTIES
		IMPORTED_LOCATION ${LIB_SSL})

	add_library(worker_sdk::c_worker_sdk STATIC IMPORTED GLOBAL)
	set_target_properties(worker_sdk::c_worker_sdk PROPERTIES
			IMPORTED_LOCATION ${LIB_IMPROBABLE_WORKER}
			INTERFACE_INCLUDE_DIRECTORIES "${WORKER_SDK_C_HEADERS}")
	target_link_libraries(worker_sdk::c_worker_sdk INTERFACE worker_sdk::RakNetLibStatic worker_sdk::ssl zlibstatic ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
endif()

