cmake_minimum_required(VERSION 3.16)
project(adventure_br_tests C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(PO_DIR ${ROOT_DIR}/po)
get_filename_component(LOCALE_DIR "${CMAKE_BINARY_DIR}/locale" ABSOLUTE)
message(STATUS "Locale directory: ${LOCALE_DIR}")

find_program(MSGFMT_EXECUTABLE msgfmt)
if(NOT MSGFMT_EXECUTABLE)
    message(FATAL_ERROR "msgfmt not found")
endif()

add_custom_command(
    OUTPUT ${LOCALE_DIR}/pt_BR/LC_MESSAGES/advent.mo
    COMMAND ${CMAKE_COMMAND} -E make_directory ${LOCALE_DIR}/pt_BR/LC_MESSAGES
    COMMAND ${MSGFMT_EXECUTABLE} -o ${LOCALE_DIR}/pt_BR/LC_MESSAGES/advent.mo ${PO_DIR}/pt_BR.po
    DEPENDS ${PO_DIR}/pt_BR.po
)

add_custom_target(translations ALL DEPENDS ${LOCALE_DIR}/pt_BR/LC_MESSAGES/advent.mo)

add_library(adventure_parser STATIC ${ROOT_DIR}/src/parser.c)
target_include_directories(adventure_parser PUBLIC ${ROOT_DIR}/src)

add_executable(parser_pt parser_commands.c)
target_include_directories(parser_pt PRIVATE ${ROOT_DIR}/src)
target_link_libraries(parser_pt adventure_parser)
target_compile_definitions(parser_pt PRIVATE PROJECT_SOURCE_DIR="${ROOT_DIR}")
add_dependencies(parser_pt translations)

enable_testing()
add_test(NAME parser_pt COMMAND parser_pt)
set_tests_properties(parser_pt PROPERTIES
    ENVIRONMENT "ADVENTURE_LOCALEDIR=${LOCALE_DIR};LANGUAGE=pt_BR;LANG=pt_BR.UTF-8;LC_ALL=pt_BR.UTF-8"
)
