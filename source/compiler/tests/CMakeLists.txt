find_package(PythonInterp 2.7)

if(PYTHONINTERP_FOUND)
  add_custom_target(pawncc_tests
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py
      -c $<TARGET_FILE:pawncc>
      -d $<TARGET_FILE:pawndisasm>
      -r $<TARGET_FILE:pawnruns>
      -i ../../../include
    DEPENDS pawncc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  file(GLOB_RECURSE meta_files "*.meta")
  foreach(meta_file IN LISTS meta_files)
    get_filename_component(test_name ${meta_file} NAME_WE)
    add_test(NAME ${test_name}
      COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py
        -c $<TARGET_FILE:pawncc>
        -d $<TARGET_FILE:pawndisasm>
        -r $<TARGET_FILE:pawnruns>
        -i ../../../include
        ${test_name}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
else()
  message("Python was not found, you will not be able to run the tests")
endif()
