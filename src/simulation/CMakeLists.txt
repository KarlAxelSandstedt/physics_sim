cmake_minimum_required(VERSION 3.14.3)

add_library(simulation STATIC 
	sim_public.h
	sim_local.h
	sim_main.c
	sim_init.c
	sim_event.c
	sim_entity.c
	)

target_link_libraries(simulation
	PUBLIC
		timer
		mg_common
		containers_lib
		memory_lib
		widget
		system
		physics
	PRIVATE
		renderer_common
)
target_include_directories(simulation
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
)

