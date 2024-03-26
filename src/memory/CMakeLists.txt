cmake_minimum_required(VERSION 3.14.3)

add_library(serialise	STATIC
	serialise.c
	serialise.h
)

target_link_libraries(serialise
	PUBLIC
		system
		memory_lib
		mg_common	
)

target_include_directories(serialise 
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(memory_lib STATIC
	mg_mempool.c
	mg_mempool.h
	mem_utils.c
	mem_utils.h
)

target_link_libraries(memory_lib
	PUBLIC
		mg_common	
		system_common
)

target_include_directories(memory_lib 
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)
