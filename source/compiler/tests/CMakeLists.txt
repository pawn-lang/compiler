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

# Compile tests
#
# These tests compare compile output against a regular expression and fail if the output
# doesn't match the expected pattern.

add_compiler_test(gh_217 ${CMAKE_CURRENT_SOURCE_DIR}/gh_217.pwn)
set_tests_properties(gh_217 PROPERTIES PASS_REGULAR_EXPRESSION "\
.*\\.pwn\\(11\\) : warning 237: user warning: this is    warning 1\
.*\\.pwn\\(13\\) : warning 237: user warning: this iswarning 2\
.*\\.pwn\\(15\\) : warning 237: user warning: this is warning 3\
.*\\.pwn\\(17\\) : warning 237: user warning: this is     warning 4\
.*\\.pwn\\(27\\) : warning 234: function is deprecated \\(symbol \"f\"\\) don't    use this     functionplease\
.*\\.pwn\\(32\\) : warning 234: function is deprecated \\(symbol \"f\"\\) don't    use this     functionplease\
")

add_compiler_test(reset_errline_gh_230 ${CMAKE_CURRENT_SOURCE_DIR}/reset_errline_gh_230.pwn)
set_tests_properties(reset_errline_gh_230 PROPERTIES PASS_REGULAR_EXPRESSION "\
.*\\.pwn\\(2\\) : error 017: undefined symbol \\\"undefined\\\"\
.*\\.pwn\\(2\\) : warning 215: expression has no effect\
.*\\.pwn\\(7\\) : warning 204: symbol is assigned a value that is never used: \\\"y\\\"\
.*\\.pwn\\(4\\) : warning 204: symbol is assigned a value that is never used: \\\"x\\\"\
")

add_compiler_test(unused_symbol_line_gh_252 ${CMAKE_CURRENT_SOURCE_DIR}/unused_symbol_line_gh_252.pwn)
set_tests_properties(unused_symbol_line_gh_252 PROPERTIES PASS_REGULAR_EXPRESSION "\
.*\\.pwn\\(4\\) : warning 203: symbol is never used: \\\"y\\\"
.*\\.pwn\\(8\\) : warning 203: symbol is never used: \\\"z\\\"
.*\\.pwn\\(1\\) : warning 203: symbol is never used: \\\"x\\\"
")

add_compiler_test(gh_283 ${CMAKE_CURRENT_SOURCE_DIR}/gh_283.pwn)
set_tests_properties(gh_283 PROPERTIES PASS_REGULAR_EXPRESSION "\
.*\\.pwn\\(5\\) : warning 234: function is deprecated \\(symbol \"print\"\\)\
")

add_compiler_test(too_many_args_crash_gh_298 ${CMAKE_CURRENT_SOURCE_DIR}/too_many_args_crash_gh_298.pwn)
set_tests_properties(too_many_args_crash_gh_298 PROPERTIES PASS_REGULAR_EXPRESSION "too many function arguments")

add_compiler_test(meaningless_class_specifiers_gh_172 ${CMAKE_CURRENT_SOURCE_DIR}/meaningless_class_specifiers_gh_172.pwn)
set_tests_properties(meaningless_class_specifiers_gh_172 PROPERTIES PASS_REGULAR_EXPRESSION "\
.*\\.pwn\\(1\\) : warning 238: meaningless combination of class specifiers \\(const reference\\)\
.*\\.pwn\\(1 \\-\\- 2\\) : warning 238: meaningless combination of class specifiers \\(const variable arguments\\)\
")

# Crashers
#
# These tests simply check that the compiler doesn't crash.
#
# TODO: Probably need to support tests that exist with a non-zero code but don't crash?
# Right now this will cause a failure.

add_compiler_test(md_array_crash_gh_220 ${CMAKE_CURRENT_SOURCE_DIR}/md_array_crash_gh_220.pwn)
