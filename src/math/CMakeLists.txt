cmake_minimum_required(VERSION 3.14.3)

add_subdirectory("debug")

add_library(physics STATIC
	rigid_body_pipeline.c
	rigid_body_pipeline.h
	rigid_body.c
	rigid_body.h
	dbvt.c
	dbvt.h
)

target_link_libraries(physics PUBLIC
	memory_lib
	containers_lib
	math_lib
	geometry
)

target_include_directories(physics INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(geometry STATIC
	geometry.c
	geometry.h
)

target_link_libraries(geometry PUBLIC
	memory_lib
	containers_lib
	math_lib
	renderer_common
)

target_include_directories(geometry INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# files to link to later
add_library(math_lib STATIC
	SSE.c
	SSE2.c
	SSE3.c
	SSE4.c
	SSE_main.h
	matrix.c
	matrix.h
	vector.c
	vector.h
	random.c
	random.h
	transformation.c
	transformation.h
	quaternion.c
	quaternion.h
	math.c
	mmath.h
)

if (UNIX) 
	target_link_libraries(math_lib 
		PUBLIC
			mg_common
			m
	)
else ()
	target_link_libraries(math_lib 
		PUBLIC
			mg_common
	)
endif ()


target_include_directories(math_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
