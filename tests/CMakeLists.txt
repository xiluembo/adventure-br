set(TEST_SOURCES
    parser_commands.c
)

add_executable(parser_pt ${TEST_SOURCES})
target_link_libraries(parser_pt PRIVATE adventure_parser)
target_compile_definitions(parser_pt PRIVATE PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_dependencies(parser_pt adventure_translations)

add_test(NAME parser_pt COMMAND parser_pt)
set_tests_properties(parser_pt PROPERTIES
    ENVIRONMENT "ADVENTURE_LOCALEDIR=${ADVENTURE_LOCALE_DIR};LANGUAGE=pt_BR;LANG=pt_BR.UTF-8;LC_ALL=pt_BR.UTF-8"
)
