function(compile_project_shaders  TARGET_NAME)

    # Locate slang compiler
    find_program(SLANGC slangc REQUIRED)

    # Shader directory
    set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders)
    set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/resources/shaders)

    if(NOT EXISTS ${SHADER_SOURCE_DIR})
        return()
    endif()

    # Collect shader sources
    file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS
        ${SHADER_SOURCE_DIR}/*.slang
    )

    set(COMPILED_SHADERS)

    foreach(SHADER ${SHADER_SOURCES})

        # Get path relative to shader root
        file(RELATIVE_PATH REL_PATH
            ${SHADER_SOURCE_DIR}
            ${SHADER}
        )

        get_filename_component(SHADER_NAME ${SHADER} NAME)

        # Output SPIR-V file
        set(SPIRV ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
        set (ENTRY_POINTS -entry vertMain -entry fragMain)

        add_custom_command(
            OUTPUT ${SPIRV}

            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_OUTPUT_DIR}

            COMMAND ${SLANGC}
                ${SHADER}
                -target spirv
                -profile spirv_1_4
                -emit-spirv-directly
                -fvk-use-entrypoint-name ${ENTRY_POINTS}
                -o ${SPIRV}

            DEPENDS ${SHADER}

            COMMENT "Compiling shader ${REL_PATH}"
        )

        list(APPEND COMPILED_SHADERS ${SPIRV})

    endforeach()

    # Create shader target
    add_custom_target(ProjectShaders ALL
        DEPENDS ${COMPILED_SHADERS}
    )

    # Ensure main target builds shaders first
    add_dependencies(${TARGET_NAME} ProjectShaders)

    # Install compiled shaders
    install(
        DIRECTORY ${CMAKE_BINARY_DIR}/resources
        DESTINATION .
    )

endfunction()