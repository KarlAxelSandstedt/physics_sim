cmake_minimum_required(VERSION 3.14.3)

add_library(math_debug STATIC
	ieee_754.c
	fINF.c
	math_debug_public.h
	math_debug_local.h
	math_debug_common.h
)

target_link_libraries(math_debug PUBLIC
	memory_lib
	mg_common
	m
)

target_include_directories(math_debug INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
