add_library(winGL INTERFACE)
target_include_directories(winGL INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(winGL INTERFACE winKHR)
