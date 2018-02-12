include_directories(..)

set(SHOVELER_SPATIALOS_COMMON_SRC
	src/log.c
	include/shoveler/spatialos/log.h
)

add_library(shoveler_spatialos_common ${SHOVELER_SPATIALOS_COMMON_SRC})
set_property(TARGET shoveler_spatialos_common PROPERTY C_STANDARD 99)

target_include_directories(shoveler_spatialos_common
		PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PUBLIC $<INSTALL_INTERFACE:include>
		PRIVATE src)

if(shoveler_use_glib)
	target_link_libraries(shoveler_spatialos_common glib)
else()
	target_link_libraries(shoveler_spatialos_common fakeglibstatic)
endif()
