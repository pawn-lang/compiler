project(pawnc)
cmake_minimum_required(VERSION 2.8)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

# check for optional include files
include(CheckIncludeFile)
check_include_file("unistd.h" HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  add_definitions(-DHAVE_UNISTD_H)
endif()
check_include_file("inttypes.h" HAVE_INTTYPES_H)
if(HAVE_INTTYPES_H)
  add_definitions(-DHAVE_INTTYPES_H)
endif()
check_include_file("stdint.h" HAVE_STDINT_H)
if(HAVE_STDINT_H)
  add_definitions(-DHAVE_STDINT_H)
endif()
check_include_file("alloca.h" HAVE_ALLOCA_H)
if(HAVE_ALLOCA_H)
  add_definitions(-DHAVE_ALLOCA_H)
endif()

# check for optional library functions
include(CheckFunctionExists)
check_function_exists(strlcpy HAVE_STRLCPY)
if(HAVE_STRLCPY)
  add_definitions(-DHAVE_STRLCPY)
endif()
check_function_exists(strlcat HAVE_STRLCAT)
if(HAVE_STRLCAT)
  add_definitions(-DHAVE_STRLCAT)
endif()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS /J)
endif()

if(UNIX)
  add_definitions(-DLINUX -DENABLE_BINRELOC -g)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../linux)
  link_libraries(pthread) #CMake adds the "lib" prefix, so "pthread" becomes "libpthread"
endif()

# Append git commit hash to the version string
include(GetGitRevisionDescription)
git_describe(GIT_COMMIT --always)
if(GIT_COMMIT)
  set(GIT_COMMIT_SUFFIX ".${GIT_COMMIT}")
else()
  set(GIT_COMMIT_SUFFIX "")
endif()
configure_file(svnrev.h.in svnrev.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# The Pawn compiler shared library
set(PAWNC_SRCS sc1.c sc2.c sc3.c sc4.c sc5.c sc6.c sc7.c
	scexpand.c sci18n.c sclist.c scmemfil.c scstate.c scvars.c
	lstring.c memfile.c libpawnc.c)
set_source_files_properties(sc1.c COMPILE_FLAGS -DNO_MAIN)
if(WIN32)
  set(PAWNC_SRCS ${PAWNC_SRCS} libpawnc.rc)
  set_source_files_properties(libpawnc.c COMPILE_FLAGS -DPAWNC_DLL)
  if(BORLAND)
    # Borland linker uses a DEF file if one is in the output directory
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpawnc.def.borland ${CMAKE_BINARY_DIR}/pawnc.def COPY_ONLY)
  else()
    # Microsoft Visual C/C++ supports a DEF file as if it were a source file
    set(PAWNC_SRCS ${PAWNC_SRCS} libpawnc.def)
  endif()
endif()
if(UNIX)
  set(PAWNC_SRCS ${PAWNC_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/../linux/binreloc.c)
endif()
add_library(pawnc SHARED ${PAWNC_SRCS})
if(WATCOM)  #Watcom C/C++ does not support a .DEF file for the exports
  set_target_properties(pawnc PROPERTIES LINK_FLAGS "/exp=libpawnc")
endif()

# The Pawn compiler driver (console program)
set(PAWNCC_SRCS pawncc.c)
if(WIN32)
  set(PAWNCC_SRCS ${PAWNCC_SRCS} libpawnc.rc)
  if(BORLAND)
    # Borland linker uses a DEF file if one is in the output directory
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pawncc.def.borland ${CMAKE_BINARY_DIR}/pawncc.def COPY_ONLY)
  else()
    # Microsoft Visual C/C++ supports a DEF file as if it were a source file
    set(PAWNC_SRCS ${PAWNC_SRCS} pawncc.def)
  endif()
endif()
add_executable(pawncc ${PAWNCC_SRCS})
target_link_libraries(pawncc pawnc)

# The Pawn disassembler
set(PAWNDISASM_SRCS pawndisasm.c ../amx/amxdbg.c ../amx/amx.c)
add_executable(pawndisasm ${PAWNDISASM_SRCS})
if(UNIX)
  target_link_libraries(pawndisasm dl)
endif()
