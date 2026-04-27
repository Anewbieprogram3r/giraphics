include (FetchContent)
function(importDependencies)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    # We only need the library, not the GLFW tests/examples
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
    find_package(Vulkan REQUIRED)
    if(NOT Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan SDK not found. "
            "Please install from https://vulkan.lunarg.com/sdk/home")
    endif()
    message(STATUS "Vulkan found: ${Vulkan_INCLUDE_DIRS}")
endfunction()
