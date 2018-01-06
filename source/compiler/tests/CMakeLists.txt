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

#
# Compile tests
#
# These tests compare compile output against a regular expression and fail if the output
# doesn't match the expected pattern.
#
add_compiler_test(gh_217 ${CMAKE_CURRENT_SOURCE_DIR}/gh_217.pwn)
set_tests_properties(gh_217 PROPERTIES PASS_REGULAR_EXPRESSION
".*: warning 237: user warning: this is    warning 1[\r\n]+\
.*: warning 237: user warning: this iswarning 2[\r\n]+\
.*: warning 237: user warning: this is warning 3[\r\n]+\
.*: warning 237: user warning: this is     warning 4[\r\n]+\
.*: warning 234: function is deprecated \\(symbol \"f\"\\) don't    use this     functionplease[\r\n]+\
.*: warning 234: function is deprecated \\(symbol \"f\"\\) don't    use this     functionplease")

add_compiler_test(macro_output "${CMAKE_CURRENT_SOURCE_DIR}/macro_output.pwn;-l;-m")
set_tests_properties(macro_output PROPERTIES PASS_REGULAR_EXPRESSION 
".* MACRO\\(A->B\\) INPUT\\(A\\) OUTPUT\\(B\\)[\r\n]+\
.* MACRO\\(B->C\\) INPUT\\(B\\) OUTPUT\\(C\\)[\r\n]+\
.* MACRO\\(C->D\\) INPUT\\(C\\) OUTPUT\\(D\\)")


#
# Crashers
#
# These tests simply check that the compiler doesn't crash.
#
# TODO: Probably need to support tests that exist with a non-zero code but don't crash?
# Right now this will cause a failure.
#
add_compiler_test(md_array_crash_gh_220 ${CMAKE_CURRENT_SOURCE_DIR}/md_array_crash_gh_220.pwn)
