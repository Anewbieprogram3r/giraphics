include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
    if(NOT TARGET glfw)  # Guard to prevent multiple inclusion
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        # FetchContent_MakeAvailable populates the content and calls
        # add_subdirectory() on it. It replaces the deprecated pattern of
        # FetchContent_Populate() + add_subdirectory() (CMake 3.30+, CMP0169).
        FetchContent_MakeAvailable(glfw)

        include_directories(${glfw_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to find vulkan SDK
macro(find_vulkan)
    # 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
    if (DEFINED VULKAN_SDK_PATH)
        set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
        set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
        set(Vulkan_FOUND "True")
    else()
        find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
        message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
        include_directories(/Users/ps/VulkanSDK/1.3.268.1/macOS/include)
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