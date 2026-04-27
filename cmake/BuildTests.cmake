#BuildTests.cmake
# Helper to link Engine + third-party libs to any target

function (linkEngineLibraries TARGET)
    target_link_libraries(${TARGET}
        PRIVATE
            Engine
            glfw
            Vulkan::Vulkan
    )
endfunction()
