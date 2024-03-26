cmake_minimum_required(VERSION 3.14.3)


add_library(mgX11 STATIC
	x11_public.h
	x11_local.h
	x11_input.c
	x11_window.c
	x11_event.c
	x11_gl.c
	)

target_include_directories(mgX11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mgX11 
	PRIVATE
		X11
		dl
		mgl
	)

