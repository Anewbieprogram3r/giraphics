#include <Engine/Graphics/Vulkan/Device.hpp>
#include <Engine/Graphics/Vulkan/Swapchain.hpp>
#include <Engine/Graphics/Window.hpp>
#include <iostream>
#include <memory>

using namespace giraphics;

class HelloWorld {
public:
    HelloWorld() = default;
    ~HelloWorld() = default;

    void onInit() {
        m_Window = std::make_unique<Window>("Hello World!", 1200, 800);
        m_Device = std::make_unique<Device>(*m_Window);
        m_Device->m_areValidationLayersAvailable = m_Device->isRequestedValidationLayersAvailable();
        m_Device->createVulkanInstance();
        m_Device->enumerateInstanceExtensions();
        m_Device->enableDebuggingCapabilities();
        m_Device->createWindowSurface();
        m_Device->enumerateAndSelectPhysicalDevice();
        m_Device->createLogicalDeviceAndConfigureQueues();
        m_Device->createCommandPool();

        VkExtent2D windowExtent{ m_Window->width(), m_Window->height() };

        m_SwapChain = std::make_unique<SwapChain>(*m_Device, windowExtent);
        m_SwapChain->createSwapChain(nullptr);
        m_SwapChain->createColorBufferImages();
        m_SwapChain->createDepthBufferImages();
        m_SwapChain->createRenderPass();
        m_SwapChain->setupFramebuffer();
        m_SwapChain->setupSynchronizationObjects();
    }

    void onExit() {
    }

    void onRun() {
        while (!m_Window->shouldClose()) {
            m_Window->pollEvents();
        }
        vkDeviceWaitIdle(m_Device->device());
    }

private:
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Device> m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
};

int main() {
    try {
        HelloWorld app{};
        app.onInit();
        app.onRun();
        app.onExit();
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}