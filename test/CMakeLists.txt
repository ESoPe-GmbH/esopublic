cmake_minimum_required(VERSION 3.5)
include_directories("config")
include_directories("../source")

add_compile_definitions(ESOPUBLICTEST)

file(GLOB_RECURSE sources "${PROJECT_SOURCE_DIR}/source/*.c")
file(GLOB tests "${PROJECT_SOURCE_DIR}/test/*.cpp")
list(REMOVE_ITEM tests "${PROJECT_SOURCE_DIR}/test/main.cpp")

foreach(file ${tests})
  set(name)
  get_filename_component(name ${file} NAME_WE)
  add_executable("${name}_tests"
    ${sources}
    ${file}
    "${PROJECT_SOURCE_DIR}/test/main.cpp")
  if(WIN32)
    target_link_libraries("${name}_tests" wsock32 ws2_32)
  endif()
  target_link_libraries("${name}_tests" gtest_main gmock)
  add_test(NAME ${name} COMMAND "${name}_tests")
  set(test_define TEST_${name})
  string(TOUPPER ${test_define} test_define)
  target_compile_definitions("${name}_tests" PUBLIC ${test_define})
endforeach()