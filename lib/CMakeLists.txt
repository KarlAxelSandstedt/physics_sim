cmake_minimum_required(VERSION 3.14.3)

add_library(stb_truetype INTERFACE)
target_include_directories(stb_truetype INTERFACE "${LIB_PATH}/stb_truetype")


