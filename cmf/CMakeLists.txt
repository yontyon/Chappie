cmf_generate_cpp(header cpp chappie::messages chappie.cmf)
add_library(chappie_messages ${cpp})
set_target_properties(chappie_messages PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(chappie_messages PUBLIC ${CMAKE_CURRENT_BINARY_DIR})