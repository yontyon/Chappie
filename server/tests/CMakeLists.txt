find_package(GTest REQUIRED)

add_library(tests_common common.hpp)
target_link_libraries(tests_common PUBLIC kvbc concordbft_storage chappie_server_headers GTest::GTest)

add_executable(commandHandler_tests commandHandler_tests.cpp)
add_test(commandHandler_tests commandHandler_tests)

target_link_libraries(commandHandler_tests
        GTest::GTest
        chappie_execution
        tests_common)

add_executable(configParser_test configParser_test.cpp)
add_test(configParser_test configParser_test)

target_link_libraries(configParser_test
        stdc++fs
        GTest::GTest
        chappie_configuration
        chappie_server_headers)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION  ${CMAKE_CURRENT_BINARY_DIR})