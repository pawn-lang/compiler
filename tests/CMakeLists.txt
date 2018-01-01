set(DEFAULT_COMPILER_OPTIONS
    -i${CMAKE_SOURCE_DIR}/include
    "-\;+"
    "-(+")

function(add_compiler_test test_name options)
    add_test(NAME ${test_name}
        COMMAND $<TARGET_FILE:pawncc> ${DEFAULT_COMPILER_OPTIONS} ${options})
    set_tests_properties(${test_name} PROPERTIES
        ENVIRONMENT PATH=$<TARGET_FILE_DIR:pawnc>)
endfunction()

add_subdirectory(compiler)