cmake_minimum_required (VERSION 3.2)
project(chappie_execution VERSION 0.1.0.0 LANGUAGES CXX)

add_library(chappie_execution 
            chappieReplica.cpp
            commandHandler.cpp)
target_include_directories(chappie_execution PUBLIC .)
target_link_libraries(chappie_execution PUBLIC 
                        kvbc 
                        corebft 
                        stdc++fs 
                        chappie_messages 
                        chappie_server_headers
                        chappie_configuration)

