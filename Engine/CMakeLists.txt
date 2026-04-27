add_library(Engine STATIC
    Graphics/Window/Window.cpp
    Graphics/Window/Glfw/GlfwWindow.cpp
)

target_include_directories(Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(Engine
    PUBLIC
        glfw
        Vulkan::Vulkan
)