cmake_minimum_required(VERSION 3.14.3)

add_executable(all_tests 
	test_main.c 
	test_container.c
	test_math.c
	test_macro.h
	test_common.h 
	test_public.h)

target_link_libraries(all_tests PRIVATE 
	containers_lib
	math_lib
	physics
	math_debug) 

add_test(NAME "all_tests" COMMAND $<TARGET_FILE:all_tests>)
