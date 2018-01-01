cmake_minimum_required(VERSION 2.8.6)

include(CTest)

add_subdirectory(source/compiler)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()