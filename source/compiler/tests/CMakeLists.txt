find_package(PythonInterp 3.0 REQUIRED)

add_custom_target(pawncc_tests COMMAND
  COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py
    -c $<TARGET_FILE:pawncc>
    -i ${CMAKE_SOURCE_DIR}/include
  DEPENDS pawncc
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
