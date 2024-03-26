cmake_minimum_required(VERSION 3.14.3)

# files to link to later
add_library(containers_lib STATIC
	mg_buffer.h
	bitstream.c
	bitstream.h
	dynamic_array.c
	dynamic_array.h
	list.c
	list.h
	hash_index.c
	hash_index.h
	hierarchy.c
	hierarchy.h
	queue.c
	queue.h
	bit_vector.c
	bit_vector.h
	mg_string.c
	mg_string.h
	ring_buffer.c
	ring_buffer.h
	relation_list.c
	relation_list.h
	containers.h
	array_list.c
	array_list.h
)

if (UNIX)
	target_link_libraries(containers_lib
		PUBLIC
			mg_common
			memory_lib
			thread
		PRIVATE
			math_lib
			rt # posix IPC
	)
elseif (WIN32)
	target_link_libraries(containers_lib
		PUBLIC
			mg_common
			memory_lib
		PRIVATE
			math_lib
	)
endif ()


target_include_directories(containers_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
