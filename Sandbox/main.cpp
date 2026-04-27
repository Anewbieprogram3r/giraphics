#include <Graphics/Window/Glfw/GlfwWindow.hpp>
#include <iostream>
#include <memory>

int main()
{
    std::cout << "Starting Giraphics Sandbox...\n";
    try
    {
        // Create an OS window via GLFW (800x600, Vulkan-ready, non-resizable)
        auto window = std::make_unique<giraphics::GlfwWindow>("giraphics", 800, 600);
        std::cout << "Window is open. Close it to exit.\n";
        // ── Main loop ─────────────────────────────────────────────────────
        // Later lectures will add:
        //   - VkInstance creation
        //   - VkSurfaceKHR from window->nativeHandle()
        //   - Swap chain setup
        //   - Command buffer recording & submission
        while(!window->shouldClose()) {
            window->pollEvents();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "[ERROR] " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    std::cout << "Exiting Giraphics Sandbox. Goodbye!\n";
    return EXIT_SUCCESS;
}