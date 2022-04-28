cmake_minimum_required (VERSION 3.2)
project(chappie_configuration VERSION 0.1.0.0 LANGUAGES CXX)

add_library(chappie_configuration 
            chappieConfiguration.cpp)
target_include_directories(chappie_configuration PUBLIC .)
target_link_libraries(chappie_configuration PUBLIC yaml-cpp chappie_server_headers bftcommunication)

