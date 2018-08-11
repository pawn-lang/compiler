project(pawn C)
cmake_minimum_required(VERSION 2.8)

# For the compiler we only utilize headers from source/amx, i.e. we don't
# actually build any of the AMX modules.
# add_subdirectory(source/amx)

add_subdirectory(source/compiler)
