function(install_resources  TARGET_NAME)

    set(TEXTURES_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures)

    if(NOT EXISTS ${TEXTURES_SOURCE_DIR})
        return()
    endif()

    add_custom_target(${TARGET_NAME}_copy_textures ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TEXTURES_SOURCE_DIR}"
            "${CMAKE_BINARY_DIR}/resources/textures"
        VERBATIM
    )
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_copy_textures)

    install(
        DIRECTORY ${TEXTURES_SOURCE_DIR}
        DESTINATION resources
    )
endfunction()