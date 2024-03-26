cmake_minimum_required(VERSION 3.14.3)

add_library(profiler STATIC profiler.c profiler.h)
target_link_libraries(profiler
	PRIVATE
		timer
	PUBLIC
		mg_common
		containers_lib
)
target_include_directories(profiler
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(decoders STATIC decode.c decode.h)
target_link_libraries(decoders PUBLIC containers_lib memory_lib)
target_include_directories(decoders INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(font_lib STATIC mg_font.c mg_font.h)
target_include_directories(font_lib
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/sys"
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(font_lib
	PUBLIC
		mg_common
		memory_lib
		math_lib
	PRIVATE
		stb_truetype
)

add_library(sort_lib STATIC sort.c sort.h)
target_link_libraries(sort_lib
	PUBLIC
		memory_lib
		mg_common
)
target_include_directories(sort_lib
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(widget STATIC widget.c widget.h)
target_link_libraries(widget
	PRIVATE
		sort_lib
		font_lib	
	PUBLIC
		memory_lib
		mg_common	
		math_lib
		containers_lib
)
target_include_directories(widget
	INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)
