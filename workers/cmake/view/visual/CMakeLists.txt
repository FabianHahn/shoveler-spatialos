include_directories(..)

set(SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_SRC
	src/client.c
	src/drawables.c
	src/light.c
	src/model.c
	include/shoveler/spatialos/worker/view/visual/client.h
	include/shoveler/spatialos/worker/view/visual/controller.h
	include/shoveler/spatialos/worker/view/visual/drawables.h
	include/shoveler/spatialos/worker/view/visual/light.h
	include/shoveler/spatialos/worker/view/visual/model.h
	include/shoveler/spatialos/worker/view/visual/scene.h
)

add_library(shoveler_spatialos_worker_view_visual ${SHOVELER_SPATIALOS_WORKER_VIEW_VISUAL_SRC})
set_property(TARGET shoveler_spatialos_worker_view_visual PROPERTY C_STANDARD 99)
target_link_libraries(shoveler_spatialos_worker_view_visual shoveler_spatialos_worker_view_base shoveler)

target_include_directories(shoveler_spatialos_worker_view_visual
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PUBLIC $<INSTALL_INTERFACE:include>
		PRIVATE src)
