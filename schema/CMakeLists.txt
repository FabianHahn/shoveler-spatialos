if(MSVC)
	set(SCHEMA_COMPILER ${CMAKE_BINARY_DIR}/external/schema_compiler/windows/schema_compiler.exe)
else()
	set(SCHEMA_COMPILER ${CMAKE_BINARY_DIR}/external/schema_compiler/linux/schema_compiler)
endif()

set(STANDARD_LIBRARY_SCHEMA_DIR ${CMAKE_BINARY_DIR}/external/schema)

set(SCHEMA_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/shoveler.schema
	${STANDARD_LIBRARY_SCHEMA_DIR}/improbable/standard_library.schema
	${STANDARD_LIBRARY_SCHEMA_DIR}/improbable/restricted/system_components.schema
)

set(SCHEMA_BUNDLE ${CMAKE_CURRENT_BINARY_DIR}/schema.sb)

add_custom_command(OUTPUT ${SCHEMA_BUNDLE}
	DEPENDS ${SCHEMA_SRC}
	COMMAND ${SCHEMA_COMPILER}
		--schema_path=${CMAKE_CURRENT_SOURCE_DIR}
		--schema_path=${STANDARD_LIBRARY_SCHEMA_DIR}
		--bundle_out=${SCHEMA_BUNDLE}
		--load_all_schema_on_schema_path
		${SCHEMA_SRC}
)

add_custom_target(schema_bundle ALL
	DEPENDS ${SCHEMA_BUNDLE})