function(adventure_add_translations TARGET_NAME)
    set(options)
    set(oneValueArgs DOMAIN OUTPUT_DIR)
    set(multiValueArgs PO_FILES OUT_MO_FILES)
    cmake_parse_arguments(ADVENTURE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT ADVENTURE_DOMAIN)
        message(FATAL_ERROR "adventure_add_translations: DOMAIN argument is required")
    endif()
    if(NOT ADVENTURE_OUTPUT_DIR)
        message(FATAL_ERROR "adventure_add_translations: OUTPUT_DIR argument is required")
    endif()
    if(NOT ADVENTURE_PO_FILES)
        set(${ADVENTURE_OUT_MO_FILES} "" PARENT_SCOPE)
        add_custom_target(${TARGET_NAME})
        return()
    endif()

    set(mo_files)
    foreach(po IN LISTS ADVENTURE_PO_FILES)
        get_filename_component(lang "${po}" NAME_WE)
        set(locale_dir "${ADVENTURE_OUTPUT_DIR}/${lang}/LC_MESSAGES")
        set(mo "${locale_dir}/${ADVENTURE_DOMAIN}.mo")
        add_custom_command(
            OUTPUT "${mo}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${locale_dir}"
            COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o "${mo}" "${po}"
            DEPENDS "${po}"
            COMMENT "Compiling ${lang} translation"
            VERBATIM
        )
        list(APPEND mo_files "${mo}")
    endforeach()

    if(mo_files)
        add_custom_target(${TARGET_NAME} ALL DEPENDS ${mo_files})
    else()
        add_custom_target(${TARGET_NAME})
    endif()

    if(ADVENTURE_OUT_MO_FILES)
        set(${ADVENTURE_OUT_MO_FILES} "${mo_files}" PARENT_SCOPE)
    endif()
endfunction()
