include_directories(..)

set(SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_SRC
	src/light.c
	src/model.c
)

add_library(shoveler_spatialos_worker_view_visual ${SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_SRC})
set_property(TARGET shoveler_spatialos_worker_view_visual PROPERTY C_STANDARD 99)
target_link_libraries(shoveler_spatialos_worker_view_visual shoveler_spatialos_worker_view_base shoveler)

target_include_directories(shoveler_spatialos_worker_view_visual
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PUBLIC $<INSTALL_INTERFACE:include>
		PRIVATE src)
