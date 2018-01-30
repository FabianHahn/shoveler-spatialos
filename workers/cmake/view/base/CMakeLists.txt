include_directories(..)

set(SHOVELER_SPATIALOS_WORKER_VIEW_BASE_SRC
	src/view.c
)

add_library(shoveler_spatialos_worker_view_base ${SHOVELER_SPATIALOS_WORKER_VIEW_BASE_SRC})
set_property(TARGET shoveler_spatialos_worker_view_base PROPERTY C_STANDARD 99)

target_include_directories(shoveler_spatialos_worker_view_base
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PUBLIC $<INSTALL_INTERFACE:include>
		PRIVATE src)

if(shoveler_use_glib)
	target_link_libraries(shoveler_spatialos_worker_view_base glib)
else()
	target_link_libraries(shoveler_spatialos_worker_view_base fakeglibstatic)
endif()