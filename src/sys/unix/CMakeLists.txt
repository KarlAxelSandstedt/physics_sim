cmake_minimum_required(VERSION 3.14.3)

add_subdirectory("x11")

add_library(unix_interface STATIC
	unix_local.h
	unix_public.h
	unix_IO.c
	)

target_include_directories(unix_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(unix_interface 
	PUBLIC
		containers_lib
		memory_lib
	)
