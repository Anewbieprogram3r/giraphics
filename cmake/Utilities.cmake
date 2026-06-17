# Define a macro to group source and header files by directory
macro(GROUP_FILES_BY_FOLDER all_files)
    foreach(FILE ${ALL_FILES})
        # Get the directory of the file
        get_filename_component(PARENT_DIR "${FILE}" PATH)

        # Replace '/' with '\' for Visual Studio group formatting
        string(REPLACE "/" "\\" GROUP "${PARENT_DIR}")

        # Classify files into Source or Header groups based on their extension
        if ("${FILE}" MATCHES ".*\\.cpp")
        set(GROUP "Source Files\\${GROUP}")
        elseif("${FILE}" MATCHES ".*\\.hpp")
        set(GROUP "Header Files\\${GROUP}")
        endif()

        # Add the file to the corresponding source group
        source_group("${GROUP}" FILES "${FILE}")
    endforeach()
endmacro()

function(compileShaders)
    find_program(GLSL_VALIDATOR glslangValidator HINTS
    ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}
      /usr/bin
      /usr/local/bin
      ${VULKAN_SDK_PATH}/Bin
      ${VULKAN_SDK_PATH}/Bin32
      $ENV{VULKAN_SDK}/Bin/
      $ENV{VULKAN_SDK}/Bin32/
    )

    file(GLOB_RECURSE SHADER_SOURCES
    "${PROJECT_SOURCE_DIR}/Engine/Assets/Shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/Engine/Assets/Shaders/*.vert"
    )

    foreach(GLSL ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/Engine/Assets/Shaders/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(GLSL)
    add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
    )
endfunction()