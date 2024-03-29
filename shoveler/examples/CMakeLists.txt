set(SHOVELER_EXAMPLE_CANVAS_SRC
	canvas.c
)

set(SHOVELER_EXAMPLE_CANVAS_LAYERS_SRC
	canvas_layers.c
)

set(SHOVELER_EXAMPLE_CLIENT_SRC
	client.c
)

set(SHOVELER_EXAMPLE_CLIENT_TEXT_SRC
	client_text.c
)

set(SHOVELER_EXAMPLE_LIGHTS_SRC
	lights.c
)

set(SHOVELER_EXAMPLE_TILES_SRC
	tiles.c
)

if(SHOVELER_BUILD_EXAMPLES)
	add_executable(shoveler_example_canvas ${SHOVELER_EXAMPLE_CANVAS_SRC})
	set_property(TARGET shoveler_example_canvas PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_canvas shoveler::shoveler_opengl)

	add_executable(shoveler_example_canvas_layers ${SHOVELER_EXAMPLE_CANVAS_LAYERS_SRC})
	set_property(TARGET shoveler_example_canvas_layers PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_canvas_layers shoveler::shoveler_opengl)

	add_executable(shoveler_example_client ${SHOVELER_EXAMPLE_CLIENT_SRC})
	set_property(TARGET shoveler_example_client PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_client shoveler::shoveler_client)

	add_executable(shoveler_example_client_text ${SHOVELER_EXAMPLE_CLIENT_TEXT_SRC})
	set_property(TARGET shoveler_example_client_text PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_client_text shoveler::shoveler_client)

	add_executable(shoveler_example_font font.c)
	set_property(TARGET shoveler_example_font PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_font shoveler::shoveler_base)

	add_executable(shoveler_example_lights ${SHOVELER_EXAMPLE_LIGHTS_SRC})
	set_property(TARGET shoveler_example_lights PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_lights shoveler::shoveler_opengl)

	add_executable(shoveler_example_text text.c)
	set_property(TARGET shoveler_example_text PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_text shoveler::shoveler_opengl)

	add_executable(shoveler_example_tiles ${SHOVELER_EXAMPLE_TILES_SRC})
	set_property(TARGET shoveler_example_tiles PROPERTY C_STANDARD 11)
	target_link_libraries(shoveler_example_tiles shoveler::shoveler_opengl)

	if(SHOVELER_INSTALL)
		install(TARGETS
				shoveler_example_canvas
				shoveler_example_canvas_layers
				shoveler_example_client
				shoveler_example_client_text
				shoveler_example_font
				shoveler_example_lights
				shoveler_example_text
				shoveler_example_tiles
			EXPORT shoveler-targets
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
			RUNTIME DESTINATION bin
			INCLUDES DESTINATION include)
	endif()
endif()

