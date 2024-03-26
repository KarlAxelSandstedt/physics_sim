cmake_minimum_required(VERSION 3.14.3)

add_library(system_common INTERFACE)
target_include_directories(system_common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(system_common
	INTERFACE	
		mg_common	
		containers_lib
	)

add_library(thread 
	STATIC 
		thread.h
		thread.c
		)

target_include_directories(thread
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)


add_library(timer 
	STATIC 
		timer.h
		timer.c
		)

target_link_libraries(timer
	PRIVATE
		system
	PUBLIC	
		mg_common	
		containers_lib
	)

target_include_directories(timer
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(system 
	STATIC
		system_local.h
		system_public.h
		thread.c
		system_IO.c
		system_input.c
		system_event_queue.c
		system_init.c
		graphics_context.c
	)

target_include_directories(system
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(system
	PUBLIC
		mg_common	
		system_common
		containers_lib
		memory_lib
		math_lib
	PRIVATE
		timer
)

target_link_libraries(thread PRIVATE Threads::Threads)

if (UNIX)
	target_link_libraries(system PUBLIC mgX11 PRIVATE unix_interface)
	target_link_libraries(thread PRIVATE unix_interface)
elseif (WIN32)
	target_link_libraries(system PUBLIC mgWin)
	target_link_libraries(thread PUBLIC mgWin)
endif ()
