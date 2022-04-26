cmake_minimum_required (VERSION 3.2)
project(chappie_server VERSION 0.1.0.0 LANGUAGES CXX)


add_library(chappie_server_headers INTERFACE)
target_include_directories(chappie_server_headers INTERFACE include)

add_subdirectory(src/consensus)
add_subdirectory(tests)