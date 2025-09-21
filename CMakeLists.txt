cmake_minimum_required(VERSION 3.16)

project(adventure VERSION 4.2 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

option(ADVENTURE_ENABLE_GETTEXT "Enable gettext-based localization" ON)
option(ADVENTURE_STRICT_2WORD_INPUT "Limit parsing to two significant words" ON)
option(ADVENTURE_WARN_AS_ERROR "Treat compiler warnings as errors" OFF)

set(ADVENTURE_PROGRAM_SUFFIX "" CACHE STRING "Suffix appended to the adventure binary name")
set(ADVENTURE_PACKAGE advent)

if(ADVENTURE_ENABLE_GETTEXT)
    find_package(Gettext REQUIRED)
    find_package(Intl REQUIRED)
    if(NOT GETTEXT_MSGFMT_EXECUTABLE)
        message(FATAL_ERROR "msgfmt executable not found; disable ADVENTURE_ENABLE_GETTEXT to build without translations")
    endif()
    set(HAVE_GETTEXT 1)
else()
    set(HAVE_GETTEXT 0)
endif()

set(ADVENTURE_DATA_INSTALL_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${ADVENTURE_PACKAGE}")
set(ADVENTURE_LOCALE_INSTALL_DIR "${CMAKE_INSTALL_FULL_LOCALEDIR}")
file(TO_CMAKE_PATH "${ADVENTURE_DATA_INSTALL_DIR}" ADVENTURE_DATA_INSTALL_DIR)
file(TO_CMAKE_PATH "${ADVENTURE_LOCALE_INSTALL_DIR}" ADVENTURE_LOCALE_INSTALL_DIR)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(adventure_project_options INTERFACE)

target_compile_definitions(adventure_project_options INTERFACE HAVE_CONFIG_H)

if(ADVENTURE_ENABLE_GETTEXT)
    target_compile_definitions(adventure_project_options INTERFACE HAVE_GETTEXT)
    if(Intl_INCLUDE_DIRS)
        target_include_directories(adventure_project_options INTERFACE ${Intl_INCLUDE_DIRS})
    endif()
    if(Intl_LIBRARIES)
        target_link_libraries(adventure_project_options INTERFACE ${Intl_LIBRARIES})
    endif()
endif()

target_include_directories(adventure_project_options INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(adventure_project_options INTERFACE
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>>:-Wall;-Wextra;-Wpedantic;-funsigned-char>
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:MSVC>>:/W4;/utf-8;/J>
)

target_compile_definitions(adventure_project_options INTERFACE
    $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN;NOMINMAX;_CRT_SECURE_NO_WARNINGS;_CRT_NONSTDC_NO_WARNINGS;_CRT_DECLARE_NONSTDC_NAMES;_WIN32_WINNT=0x0600>
)

if(ADVENTURE_WARN_AS_ERROR)
    target_compile_options(adventure_project_options INTERFACE
        $<$<AND:$<COMPILE_LANGUAGE:C>,$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>>>:-Werror>
        $<$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:MSVC>>:/WX>
    )
endif()

set(ADVENTURE_PROGRAM_OUTPUT_NAME "advent${ADVENTURE_PROGRAM_SUFFIX}")
set(SUFFIX "${ADVENTURE_PROGRAM_SUFFIX}${CMAKE_EXECUTABLE_SUFFIX}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/advent.desktop.in
               ${CMAKE_CURRENT_BINARY_DIR}/advent.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/advent.desktop
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)

set(adventure_sources
    src/advent.c
    src/database.c
    src/english.c
    src/itverb.c
    src/turn.c
    src/verb.c
)

add_library(adventure_parser STATIC src/parser.c)
target_link_libraries(adventure_parser PUBLIC adventure_project_options)

add_executable(advent ${adventure_sources})
set_target_properties(advent PROPERTIES OUTPUT_NAME "${ADVENTURE_PROGRAM_OUTPUT_NAME}")
target_link_libraries(advent PRIVATE adventure_parser adventure_project_options)

add_executable(advent0 src/advent0.c)
target_link_libraries(advent0 PRIVATE adventure_project_options)

set(adventure_data_files
    src/advent1.txt
    src/advent2.txt
    src/advent3.txt
    src/advent4.txt
)

install(TARGETS advent
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${adventure_data_files}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${ADVENTURE_PACKAGE})

if(ADVENTURE_ENABLE_GETTEXT)
    include(cmake/Translations.cmake)
    file(GLOB PO_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/po/*.po")
    set(ADVENTURE_LOCALE_DIR "${CMAKE_BINARY_DIR}/locale")
    file(MAKE_DIRECTORY "${ADVENTURE_LOCALE_DIR}")
    adventure_add_translations(adventure_translations
        DOMAIN ${ADVENTURE_PACKAGE}
        OUTPUT_DIR ${ADVENTURE_LOCALE_DIR}
        PO_FILES ${PO_FILES}
        OUT_MO_FILES ADVENTURE_MO_FILES)
    foreach(po IN LISTS PO_FILES)
        get_filename_component(lang "${po}" NAME_WE)
        install(FILES "${ADVENTURE_LOCALE_DIR}/${lang}/LC_MESSAGES/${ADVENTURE_PACKAGE}.mo"
                DESTINATION ${CMAKE_INSTALL_LOCALEDIR}/${lang}/LC_MESSAGES)
    endforeach()
else()
    add_custom_target(adventure_translations)
    set(ADVENTURE_LOCALE_DIR "${CMAKE_BINARY_DIR}/locale")
    file(MAKE_DIRECTORY "${ADVENTURE_LOCALE_DIR}")
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

set(CPACK_PACKAGE_NAME ${ADVENTURE_PACKAGE})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;TXZ")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${ADVENTURE_PACKAGE}-${PROJECT_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES
    "/build/;/\\.git/;~$;\\.swp$;\\.DS_Store$")
include(CPack)
