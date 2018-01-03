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

add_compiler_test(gh_217 ${CMAKE_CURRENT_SOURCE_DIR}/gh_217.pwn)
set_tests_properties(gh_217 PROPERTIES PASS_REGULAR_EXPRESSION
".*: warning 237: user warning: this is    warning 1[\r\n]+\
.*: warning 237: user warning: this iswarning 2[\r\n]+\
.*: warning 237: user warning: this is warning 3[\r\n]+\
.*: warning 237: user warning: this is     warning 4[\r\n]+\
.*: warning 234: function is deprecated \\(symbol \"f\"\\) don't    use this     functionplease[\r\n]+\
.*: warning 234: function is deprecated \\(symbol \"f\"\\) don't    use this     functionplease")
