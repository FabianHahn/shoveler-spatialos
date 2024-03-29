include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SHOVELER_GENERATOR_SRC
	schema_generator.c
	../workers/client/spatialos_client_schema.c
	../workers/client/spatialos_client_schema.h
)

add_executable(ShovelerSchemaGenerator ${SHOVELER_GENERATOR_SRC})
target_link_libraries(ShovelerSchemaGenerator shoveler_schema shoveler_opengl PNG::PNG ZLIB::ZLIB shoveler_worker_common worker_sdk::c_worker_sdk)

set(SCHEMA_FILE ${CMAKE_CURRENT_BINARY_DIR}/shoveler.schema)

add_custom_command(OUTPUT ${SCHEMA_FILE}
	DEPENDS ShovelerSchemaGenerator
	COMMAND ShovelerSchemaGenerator ${SCHEMA_FILE}
)

if(MSVC)
	set(SCHEMA_COMPILER ${CMAKE_BINARY_DIR}/external/schema_compiler/windows/schema_compiler.exe)
elseif(APPLE)
	set(SCHEMA_COMPILER ${CMAKE_BINARY_DIR}/external/schema_compiler/macos/schema_compiler)
else()
	set(SCHEMA_COMPILER ${CMAKE_BINARY_DIR}/external/schema_compiler/linux/schema_compiler)
endif()

set(STANDARD_LIBRARY_SCHEMA_DIR ${CMAKE_BINARY_DIR}/external/schema)

set(SCHEMA_SRC
	${SCHEMA_FILE}
	${STANDARD_LIBRARY_SCHEMA_DIR}/improbable/standard_library.schema
	${STANDARD_LIBRARY_SCHEMA_DIR}/improbable/restricted/system_components.schema
)

set(SCHEMA_BUNDLE ${CMAKE_CURRENT_BINARY_DIR}/schema.sb)

add_custom_command(OUTPUT ${SCHEMA_BUNDLE}
	DEPENDS ${SCHEMA_SRC}
	COMMAND ${SCHEMA_COMPILER}
		--schema_path=${CMAKE_CURRENT_BINARY_DIR}
		--schema_path=${STANDARD_LIBRARY_SCHEMA_DIR}
		--bundle_out=${SCHEMA_BUNDLE}
		--load_all_schema_on_schema_path
		${SCHEMA_SRC}
)

add_custom_target(schema_bundle ALL
	DEPENDS ${SCHEMA_BUNDLE})
