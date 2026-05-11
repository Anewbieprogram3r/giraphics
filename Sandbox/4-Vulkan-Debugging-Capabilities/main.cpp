#include <Engine/Graphics/Window.hpp>
#include <Engine/Graphics/Vulkan/VulkanContext.hpp>
#include <Engine/Graphics/Vulkan/Device.hpp>

int main() {
    try {
        // Create a unique pointer to a Window object
        giraphics::Window engineWindow("My Engine", 800, 600);
        auto context = std::make_unique<giraphics::VulkanContext>(engineWindow);

        // Main loop
        while (!engineWindow.shouldClose()) {
            engineWindow.pollEvents();
            // Optionally, add a short sleep to reduce CPU usage
            // std::this_thread::sleep_for(std::chrono::milliseconds(16)); // Approx 60 FPS
        }
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}