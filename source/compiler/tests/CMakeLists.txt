find_package(PythonInterp 2.7)

if(PYTHONINTERP_FOUND)
  add_custom_target(pawncc_tests COMMAND
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py
      -c $<TARGET_FILE:pawncc>
      -d $<TARGET_FILE:pawndisasm>
      -r $<TARGET_FILE:pawnruns>
      -i ../../../include
    DEPENDS pawncc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
  message("Python was not found, you will not be able to run the compiler tests")
endif()
