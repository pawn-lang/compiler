project(pawn C)
cmake_minimum_required(VERSION 2.8)

set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/products)
add_subdirectory(source/compiler ${OUTPUT_DIR})

# For the compiler we only utilize headers from source/amx, i.e. we don't
# actually build any of the AMX modules.
if(BUILD_AMX)
  add_subdirectory(source/amx ${OUTPUT_DIR})
endif()
