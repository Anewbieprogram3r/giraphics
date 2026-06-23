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
        createCommandBuffers();
    }

    void onExit() {
        vkDestroyPipelineLayout(m_Device->device(), m_PipelineLayout, nullptr);
    }

    void onRun() {
        while (!m_Window->shouldClose()) {
            m_Window->pollEvents();
            render();
        }
        vkDeviceWaitIdle(m_Device->device());
    }

    void createPipeline();
    void createCommandBuffers();
    void render();

private:
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Device> m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout = nullptr;
    std::vector<VkCommandBuffer> m_CommandBuffers;
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

void HelloWorld::createCommandBuffers() 
{
    m_CommandBuffers.resize(m_SwapChain->swapChainImageCount());
    VkCommandBufferAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_Device->commandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size())
    };
    
    if (vkAllocateCommandBuffers(m_Device->device(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) 
    {throw std::runtime_error("failed to allocate command buffers!");}

    for (size_t i =0; i< m_CommandBuffers.size(); i++) 
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS) 
        {throw std::runtime_error("failed to begin recording command buffer!");}

        std::array<VkClearValue, 2> clearValues
        { {
            {.color = { {0.0f, 0.0f, 0.0f, 1.0f} } },
            {.depthStencil = {1.0f, 0} }
        } };
        
        VkRenderPassBeginInfo renderPassInfo
        {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = m_SwapChain->renderPass(),
            .framebuffer = m_SwapChain->frameBuffer(i),
            .renderArea = {.offset = {0, 0}, .extent = m_SwapChain->swapChainExtent()},
            .clearValueCount = static_cast<uint32_t>(clearValues.size()),
            .pClearValues = clearValues.data()
        };

        vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport viewport
        {
            .x =0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_SwapChain->swapChainExtent().width),
            .height = static_cast<float>(m_SwapChain->swapChainExtent().height),
            .minDepth =0.0f,
            .maxDepth = 1.0f
        };

        vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
        VkRect2D scissor{{0,0},m_SwapChain->swapChainExtent()};
        vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
        
        m_Pipeline->bind(m_CommandBuffers[i]);
        vkCmdDraw(m_CommandBuffers[i], 4, 1, 0, 0);
        vkCmdEndRenderPass(m_CommandBuffers[i]);


        if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS) 
        {throw std::runtime_error("failed to record command buffer!");}
    }
}

void HelloWorld::render() 
{
    uint32_t imageIndex;
    auto result = m_SwapChain->acquireNextImage(&imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
    }
    result = m_SwapChain->enqueueCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS) {throw std::runtime_error("failed to present swap chain image!");}
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