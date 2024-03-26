cmake_minimum_required(VERSION 3.14.3)

add_subdirectory("GL")
#add_subdirectory("GL/KHR")

add_library(mgWin STATIC
	win_public.h
	win_local.h
	win_input.c
	win_window.c
	win_event.c
	win_init.c
	win_gl.c
	win_IO.c
	)

target_link_libraries(mgWin
	PUBLIC
		mg_common
		system_common
	PRIVATE
		mgl
		opengl32
		OpenGL::GL
		)

target_include_directories(mgWin
 	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
		"GL"
		#winGL
		#winKHR
		)
