project(pawnc C)
cmake_minimum_required(VERSION 2.8)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)

set(VERSION_MAJOR 3)
set(VERSION_MINOR 10)
set(VERSION_BUILD 11)
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD})
set(VERSION_STR ${VERSION})
math(EXPR VERSION_INT "${VERSION_MAJOR} << 8 | ${VERSION_MINOR}")

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
check_include_file("endian.h" HAVE_ENDIAN_H)
if(HAVE_ENDIAN_H)
  add_definitions(-DHAVE_ENDIAN_H)
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
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
  if(MSVC LESS 1900)
    # MSVC 2013 and below don't support the "inline" keyword
    add_definitions(-Dinline=__inline)
  endif()
endif()

if(UNIX)
  add_definitions(-DLINUX -DENABLE_BINRELOC -g)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../linux)
  link_libraries(pthread)
endif()

if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# The Pawn compiler shared library
set(PAWNC_SRCS
  hashtable/wrap_hashtable.c
  hashtable/wrap_hashtable.h
  hashtable/hashtable.h
  libpawnc.c
  lstring.c
  lstring.h
  memfile.c
  memfile.h
  sc.h
  sc1.c
  sc2.c
  sc3.c
  sc4.c
  sc5.c
  sc6.c
  sc7.c
  sci18n.c
  sclist.c
  scmemfil.c
  scstate.c
  scvars.c
  version.h)
set_source_files_properties(sc1.c COMPILE_FLAGS -DNO_MAIN)
if(WIN32)
  set(PAWNC_SRCS ${PAWNC_SRCS} libpawnc.rc)
  set_source_files_properties(libpawnc.c COMPILE_FLAGS -DPAWNC_DLL)
  if(BORLAND)
    # Borland linker uses a DEF file if one is in the output directory
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpawnc.def.borland
                   ${CMAKE_CURRENT_BINARY_DIR}/pawnc.def
                   COPYONLY)
  else()
    # Microsoft Visual C/C++ supports a DEF file as if it were a source file
    set(PAWNC_SRCS ${PAWNC_SRCS} libpawnc.def)
  endif()
endif()
if(UNIX)
  set(PAWNC_SRCS
    ${PAWNC_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../linux/binreloc.c
  )
endif()
add_library(pawnc SHARED ${PAWNC_SRCS})
if(WATCOM)  #Watcom C/C++ does not support a .DEF file for the exports
  set_target_properties(pawnc PROPERTIES LINK_FLAGS "/exp=libpawnc")
elseif(MINGW)
  set_target_properties(pawnc PROPERTIES LINK_FLAGS
    "-Wl,--enable-stdcall-fixup")
  set_target_properties(pawnc PROPERTIES PREFIX "")
endif()

# The Pawn compiler driver (console program)
set(PAWNCC_SRCS pawncc.c)
if(WIN32)
  set(PAWNCC_SRCS ${PAWNCC_SRCS} libpawnc.rc)
  if(BORLAND)
    # Borland linker uses a DEF file if one is in the output directory
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pawncc.def.borland
                   ${CMAKE_CURRENT_BINARY_DIR}/pawncc.def
                   COPYONLY)
  else()
    # Microsoft Visual C/C++ supports a DEF file as if it were a source file
    set(PAWNC_SRCS ${PAWNC_SRCS} pawncc.def)
  endif()
endif()
add_executable(pawncc ${PAWNCC_SRCS})
target_link_libraries(pawncc pawnc)

# The Pawn disassembler
set(PAWNDISASM_SRCS
  pawndisasm.c
  sc.h
  ../amx/amx.h
  ../amx/amxdbg.h
  ../amx/amxdbg.c
)
add_executable(pawndisasm ${PAWNDISASM_SRCS})
if(UNIX)
  target_link_libraries(pawndisasm dl)
endif()

# Install compiler and disassembler binaries
install(TARGETS pawnc pawncc pawndisasm
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
if(MSVC)
  # If building with Microsoft Visual C++ also install corresponding
  # Program Database files (for debugging)
  install(
    FILES ${PROJECT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/pawnc.pdb
          ${PROJECT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/pawncc.pdb
          ${PROJECT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/pawndisasm.pdb
    DESTINATION bin)
endif()

# Generate targets for running compiler tests
include(CTest)
if(BUILD_TESTING)
  set(PAWNRUNS_SRCS
    pawnruns.c
    ../amx/amx.c
    ../amx/amx.h
    ../amx/amxaux.c
    ../amx/amxaux.h
    ../amx/amxcons.c
    ../amx/amxcore.c
    ../amx/amxstring.c
  )
  if(UNIX)
    set(PAWNRUNS_SRCS
      ${PAWNRUNS_SRCS}
      ../linux/getch.c
      ../linux/getch.h
      ../linux/binreloc.c
    )
  endif()
  add_executable(pawnruns ${PAWNRUNS_SRCS})
  if(UNIX)
    target_link_libraries(pawnruns dl)
  endif()
  add_subdirectory(tests)
endif()

# Generate a binary package with CPack
set(CPACK_PACKAGE_NAME pawnc)
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION ${VERSION_STR})
if(WIN32)
  set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${VERSION_STR}-windows)
elseif(APPLE)
  set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${VERSION_STR}-macos)
else()
  string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)
  set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${VERSION_STR}-${SYSTEM_NAME_LOWER})
endif()
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/../../license.txt)
include(CPack)
