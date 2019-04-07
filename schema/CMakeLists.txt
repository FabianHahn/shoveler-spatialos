if(MSVC)
	set(SCHEMA_COMPILER ${CMAKE_BINARY_DIR}/external/schema_compiler/windows/schema_compiler.exe)
else()
	set(SCHEMA_COMPILER ${CMAKE_BINARY_DIR}/external/schema_compiler/linux/schema_compiler)
endif()

set(STANDARD_LIBRARY_SCHEMA_DIR ${CMAKE_BINARY_DIR}/external/schema)
set(GENERATED_CODE_DIR ${CMAKE_BINARY_DIR}/generated)

set(SCHEMA_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/shoveler.schema
	${STANDARD_LIBRARY_SCHEMA_DIR}/improbable/standard_library.schema
)

set(SCHEMA_GENERATED_SRC
	${GENERATED_CODE_DIR}/shoveler.cc
	${GENERATED_CODE_DIR}/improbable/standard_library.cc
)

add_custom_command(OUTPUT ${SCHEMA_GENERATED_SRC}
	DEPENDS ${SCHEMA_SRC}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_CODE_DIR}
	COMMAND ${SCHEMA_COMPILER}
		--schema_path=${CMAKE_CURRENT_SOURCE_DIR}
		--schema_path=${STANDARD_LIBRARY_SCHEMA_DIR}
		--cpp_out=${GENERATED_CODE_DIR}
		--load_all_schema_on_schema_path
		${SCHEMA_SRC}
)

add_library(schema STATIC ${SCHEMA_GENERATED_SRC})
target_include_directories(schema SYSTEM PUBLIC ${GENERATED_CODE_DIR})
target_link_libraries(schema PRIVATE worker_sdk)

add_custom_command(TARGET schema POST_BUILD
	COMMAND ${SCHEMA_COMPILER}
		--schema_path=${CMAKE_CURRENT_SOURCE_DIR}
		--schema_path=${STANDARD_LIBRARY_SCHEMA_DIR}
		--descriptor_set_out=${CMAKE_CURRENT_BINARY_DIR}/schema.descriptor
		--load_all_schema_on_schema_path
		${SCHEMA_SRC}
)
