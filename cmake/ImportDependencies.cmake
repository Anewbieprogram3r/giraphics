include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
    if(NOT TARGET glfw)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(glfw)

        include_directories(${glfw_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to find vulkan SDK
macro(find_vulkan)
    if (DEFINED VULKAN_SDK_PATH)
        set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
        set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
        set(Vulkan_FOUND "True")
    else()
        find_package(Vulkan REQUIRED)
        message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
    endif()
    if (NOT Vulkan_FOUND)
        message(FATAL_ERROR "Could not find Vulkan library!")
    else()
        message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
    endif()
endmacro()

# Macro to import all dependencies
macro(importDependencies)
    import_glfw()
    find_vulkan()
endmacro()