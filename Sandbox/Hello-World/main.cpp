#include <Engine/Graphics/Vulkan/Device.hpp>
#include <Engine/Graphics/Vulkan/Swapchain.hpp>
#include <Engine/Graphics/Vulkan/Pipeline.hpp>
#include <Engine/Graphics/Window.hpp>
#include <filesystem>
#include <iostream>
#include <memory>

namespace fs = std::filesystem;
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

        m_Pipeline = std::make_unique<Pipeline>(*m_Device);

        createPipeline();
    }

    void onExit() {
        vkDestroyPipelineLayout(m_Device->device(), m_PipelineLayout, nullptr);
    }

    void onRun() {
        while (!m_Window->shouldClose()) {
            m_Window->pollEvents();
        }
        vkDeviceWaitIdle(m_Device->device());
    }

    void createPipeline();

private:
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Device> m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout = nullptr;
};

void HelloWorld::createPipeline() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    if (vkCreatePipelineLayout(m_Device->device(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    PipelineParameters params{};
    params.inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    params.renderPass = m_SwapChain->renderPass();
    params.pipelineLayout = m_PipelineLayout;

    fs::path assetFolder = fs::current_path() / "build/Assets/Shaders";
    fs::path vertexShader = assetFolder / "simple-quad.vert.spv";
    fs::path fragmentShader = assetFolder / "simple-quad.frag.spv";

    m_Pipeline->createGraphicsPipeline(vertexShader.string(), fragmentShader.string(), params);
}

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