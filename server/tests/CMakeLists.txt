find_package(GTest REQUIRED)

add_library(tests_common common.hpp)
target_link_libraries(tests_common PUBLIC kvbc concordbft_storage chappie_server_headers GTest::GTest)

add_executable(commandHandler_tests commandHandler_tests.cpp)
add_test(commandHandler_tests commandHandler_tests)

target_link_libraries(commandHandler_tests
        GTest::GTest
        chappie_execution
        tests_common)