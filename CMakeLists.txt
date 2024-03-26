cmake_minimum_required(VERSION 3.14.3)

project(basic_physics_simulation)

set(CMAKE_ASM_NASM_FLAGS_DEBUG  "-g Fdwarf")

set(SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TEST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(LIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

enable_testing()
add_subdirectory(${TEST_PATH})

if (UNIX)
	add_executable(${PROJECT_NAME} ${SRC_PATH}/sys/unix/main.c)
elseif (WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${SRC_PATH}/sys/win64/WinMain.c)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSCV")
	add_compile_options(/std:c99 /WX)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-W -Wall -g -Wno-switch -Wno-unused-parameter -Wno-unused-variable -O0)
	set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
	set(THREADS_PREFER_PTHREAD_FLAG_TRUE)
endif ()
find_package(Threads REQUIRED)

add_subdirectory("${LIB_PATH}")
add_subdirectory("${SRC_PATH}/common")
add_subdirectory("${SRC_PATH}/math")
add_subdirectory("${SRC_PATH}/containers")
add_subdirectory("${SRC_PATH}/memory")
add_subdirectory("${SRC_PATH}/misc")
add_subdirectory("${SRC_PATH}/renderer")
add_subdirectory("${SRC_PATH}/sys")
add_subdirectory("${SRC_PATH}/simulation")
link_libraries(mg_common)
link_libraries(profiler)

if (UNIX)
	add_subdirectory(${SRC_PATH}/sys/unix)
	target_link_libraries(${PROJECT_NAME}
		PRIVATE
			unix_interface
			font_lib
			memory_lib
			mgl
			Threads::Threads
			widget
			physics
			decoders
			profiler
			system
			simulation
			timer
			renderer
	)
elseif (WIN32)

	find_package(OpenGL REQUIRED)

	add_subdirectory(${SRC_PATH}/sys/win64)
	target_link_libraries(${PROJECT_NAME}
		PRIVATE
			mgWin
			system
			memory_lib
			widget
			simulation
			renderer
			)
endif ()
