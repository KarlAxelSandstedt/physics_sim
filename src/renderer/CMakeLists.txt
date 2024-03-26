cmake_minimum_required(VERSION 3.14.3)


add_library(renderer
	STATIC
		r_public.h
		r_local.h
		r_main.c
		r_init.c
		r_buffer.c
		r_camera.c
		r_primitive.c
		)

target_link_libraries(renderer
	PUBLIC
		renderer_common
		memory_lib
		math_lib
		simulation
		widget
		system
		renderer_common
	PRIVATE
		mgl
	)

target_include_directories(renderer
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
	)

add_library(renderer_common
	STATIC 
		r_common.h
		r_command.c
		mgl_primitives.c
		mgl_primitives.h
)
target_link_libraries(renderer_common
	PUBLIC
		memory_lib
		math_lib
		mgl
	)
target_include_directories(renderer_common
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
	)

add_library(mgl INTERFACE)
target_include_directories(mgl
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
)

if (UNIX)
	target_link_libraries(mgl INTERFACE mgX11 system_common)
elseif(WIN32)
	target_link_libraries(mgl INTERFACE mgWin system_common)
endif()

