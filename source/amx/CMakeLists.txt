project(pawnamx C)
cmake_minimum_required(VERSION 2.8)

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

add_definitions(-DFLOATPOINT -DFIXEDPOINT)
if(UNIX)
  add_definitions(-DLINUX)
  check_include_file("ffi.h" HAVE_FFI_H)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../linux)
endif()
if(WIN32)
  add_definitions(-DAMXEXPORT=__stdcall -DAMX_NATIVE_CALL=__stdcall -DSTDECL)
  if(NOT BORLAND)
    link_libraries(winmm)
  endif()
endif()

# --------------------------------------------------------------------------
# Extension modules

# amxArgs
set(ARGS_SRCS amxargs.c amx.c)
add_library(amxArgs SHARED ${ARGS_SRCS})
set_target_properties(amxArgs PROPERTIES PREFIX "")
if(WIN32)
  set(ARGS_SRCS ${ARGS_SRCS} dllmain.c amxargs.rc)
  if(BORLAND)
    # Borland linker uses a DEF file if one is in the output directory
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxargs.def
                   ${CMAKE_BINARY_DIR}/amxargs.def
                   COPYONLY)
  else()
    # For Microsoft Visual C/C++ we can set explicit flags for exports
    set_target_properties(amxArgs PROPERTIES LINK_FLAGS
      "/export:amx_ArgsInit /export:amx_ArgsCleanup /export:amx_ArgsSetCmdLine")
  endif()
endif()

# amxDGram
set(DGRAM_SRCS amxdgram.c amx.c)
add_library(amxDGram SHARED ${DGRAM_SRCS})
set_target_properties(amxDGram PROPERTIES PREFIX "")
if(WIN32)
  set(DGRAM_SRCS ${DGRAM_SRCS} dllmain.c amxargs.rc)
  if(BORLAND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxdgram.def
                   ${CMAKE_BINARY_DIR}/amxdgram.def
                   COPYONLY)
  else()
    set_target_properties(amxDGram PROPERTIES LINK_FLAGS
      "/export:amx_DGramInit /export:amx_DGramCleanup")
  endif()
endif()

# amxFile
set(FILE_SRCS amxfile.c amx.c)
add_library(amxFile SHARED ${FILE_SRCS})
set_target_properties(amxFile PROPERTIES PREFIX "")
if(WIN32)
  set(FILE_SRCS ${FILE_SRCS} dllmain.c amxfile.rc)
  if(BORLAND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxfile.def
                   ${CMAKE_BINARY_DIR}/amxfile.def
                   COPYONLY)
  else()
    set_target_properties(amxFile PROPERTIES LINK_FLAGS
      "/export:amx_FileInit /export:amx_FileCleanup")
  endif()
endif()

# amxFixed
set(FIXED_SRCS fixed.c amx.c)
add_library(amxFixed SHARED ${FIXED_SRCS})
set_target_properties(amxFixed PROPERTIES PREFIX "")
if(WIN32)
  set(FIXED_SRCS ${FIXED_SRCS} dllmain.c amxfixed.rc)
  if(BORLAND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxfixed.def
                   ${CMAKE_BINARY_DIR}/amxfixed.def
                   COPYONLY)
  else()
    set_target_properties(amxFixed PROPERTIES LINK_FLAGS
      "/export:amx_FixedInit /export:amx_FixedCleanup")
  endif()
endif()
if(UNIX)
  target_link_libraries(amxFixed m)
endif()

# amxFloat
set(FLOAT_SRCS float.c amx.c)
add_library(amxFloat SHARED ${FLOAT_SRCS})
set_target_properties(amxFloat PROPERTIES PREFIX "")
if(WIN32)
  set(FLOAT_SRCS ${FLOAT_SRCS} dllmain.c amxfloat.rc)
  if(BORLAND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxfloat.def
                   ${CMAKE_BINARY_DIR}/amxfloat.def
                   COPYONLY)
  else()
    set_target_properties(amxFloat PROPERTIES LINK_FLAGS
      "/export:amx_FloatInit /export:amx_FloatCleanup")
  endif()
endif()
if(UNIX)
  target_link_libraries(amxFloat m)
endif()

# amxProcess
set(PROCESS_SRCS amxprocess.c amx.c)
if(WIN32)
  add_library(amxProcess SHARED ${PROCESS_SRCS})
else()
  if(HAVE_FFI_H)
    add_library(amxProcess SHARED ${PROCESS_SRCS})
  else()
    MESSAGE(SEND_ERROR "amxProcess requires libffi,"
                       "see http://sources.redhat.com/libffi/")
    MESSAGE(SEND_ERROR "libffi is not available (foreign function interface)")
  endif()
endif()
set_target_properties(amxProcess PROPERTIES PREFIX "")
if(WIN32)
  set(PROCESS_SRCS ${PROCESS_SRCS} dllmain.c amxprocess.rc)
  if(BORLAND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxprocess.def
                   ${CMAKE_BINARY_DIR}/amxprocess.def
                   COPYONLY)
  else()
    set_target_properties(amxProcess PROPERTIES LINK_FLAGS
      "/export:amx_ProcessInit /export:amx_ProcessCleanup")
  endif()
endif()
if(UNIX)
  target_link_libraries(amxProcess dl)
endif()

# amxString
set(STRING_SRCS amxstring.c amx.c amxcons.c)
add_library(amxString SHARED ${STRING_SRCS})
set_target_properties(amxString PROPERTIES PREFIX "")
if(WIN32)
  set(STRING_SRCS ${STRING_SRCS} dllmain.c amxstring.rc)
  if(BORLAND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxstring.def
                   ${CMAKE_BINARY_DIR}/amxstring.def
                   COPYONLY)
  else()
    set_target_properties(amxString PROPERTIES LINK_FLAGS
      "/export:amx_StringInit /export:amx_StringCleanup")
  endif()
endif()

# amxTime
set(TIME_SRCS amxtime.c amx.c)
add_library(amxTime SHARED ${TIME_SRCS})
set_target_properties(amxTime PROPERTIES PREFIX "")
if(WIN32)
  set(TIME_SRCS ${TIME_SRCS} dllmain.c amxtime.rc)
  if(BORLAND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/amxtime.def
                   ${CMAKE_BINARY_DIR}/amxtime.def
                   COPYONLY)
  else()
    set_target_properties(amxTime PROPERTIES LINK_FLAGS
      "/export:amx_TimeInit /export:amx_TimeCleanup")
  endif()
endif()

# --------------------------------------------------------------------------
# Simple run-time (example program)

set(PAWNRUN_SRCS pawnrun.c amx.c amxcore.c amxcons.c amxdbg.c)
if(UNIX)
  set(PAWNRUN_SRCS
    ${PAWNRUN_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/../linux/getch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../linux/binreloc.c
  )
endif()
ADD_EXECUTABLE(pawnrun ${PAWNRUN_SRCS})
set_target_properties(pawnrun PROPERTIES
  COMPILE_FLAGS -DAMXDBG
  COMPILE_FLAGS -DENABLE_BINRELOC
)
if(UNIX)
  target_link_libraries(pawnrun dl)
endif()

# --------------------------------------------------------------------------
# Simple console debugger

set(PAWNDBG_SRCS pawndbg.c amx.c amxcore.c amxcons.c amxdbg.c)
if(UNIX)
  set(PAWNDBG_SRCS
    ${PAWNDBG_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/../linux/getch.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../linux/binreloc.c
  )
endif()
ADD_EXECUTABLE(pawndbg ${PAWNDBG_SRCS})
set_target_properties(pawndbg PROPERTIES COMPILE_FLAGS -DENABLE_BINRELOC)
if(UNIX)
  target_link_libraries(pawndbg dl)
endif()
