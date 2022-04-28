cmake_minimum_required (VERSION 3.2)
project(chappie_server VERSION 0.1.0.0 LANGUAGES CXX)

add_subdirectory(consensus)
add_subdirectory(configuration)

add_executable(chappie_replica main.cpp)
target_link_libraries(chappie_replica PUBLIC 
                    kvbc 
                    corebft 
                    chappie_configuration 
                    chappie_execution
                    bftcommunication)
