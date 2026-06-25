#include <Engine/Graphics/Vulkan/Vertex.hpp>
#include <Engine/Graphics/Vulkan/Device.hpp>
#include <Engine/Graphics/Vulkan/Swapchain.hpp>
#include <Engine/Graphics/Vulkan/Pipeline.hpp>
#include <Engine/Graphics/Window.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
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

        createDescriptorSetLayout();
        createPipeline();
        createVertexBuffer();
        createUniformBuffer();
        createDescriptorPoolAndSet();
        createCommandBuffers();
    }

    void onExit() {
        vkDestroyBuffer(m_Device->device(), m_UniformBuffer, nullptr);
        vkFreeMemory(m_Device->device(), m_UniformBufferMemory, nullptr);
        vkDestroyDescriptorPool(m_Device->device(), m_DescriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(m_Device->device(), m_DescriptorSetLayout, nullptr);
        vkDestroyBuffer(m_Device->device(), m_VertexBuffer, nullptr);
        vkFreeMemory(m_Device->device(), m_VertexBufferMemory, nullptr);
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
    void createVertexBuffer();
    void createUniformBuffer();
    void createDescriptorSetLayout();
    void createDescriptorPoolAndSet();
    void updateUniformBuffer();

private:
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Device> m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout = nullptr;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    VkBuffer m_VertexBuffer = nullptr;
    VkDeviceMemory m_VertexBufferMemory = nullptr;
    VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
    VkDescriptorPool m_DescriptorPool = nullptr;
    VkDescriptorSet m_DescriptorSet = nullptr;
    VkBuffer m_UniformBuffer = nullptr;
    VkDeviceMemory m_UniformBufferMemory = nullptr;
};

void HelloWorld::createPipeline() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &m_DescriptorSetLayout,
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
    params.bindingDescriptions = { Vertex::getBindingDescription() };
    auto attrDescs = Vertex::getAttributeDescriptions();
    params.attributeDescriptions = { attrDescs.begin(), attrDescs.end() };

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

        VkBuffer vertexBuffers[] = { m_VertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);

        vkCmdDraw(m_CommandBuffers[i], 4, 1, 0, 0);
        vkCmdEndRenderPass(m_CommandBuffers[i]);


        if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS) 
        {throw std::runtime_error("failed to record command buffer!");}
    }
}

void HelloWorld::render() 
{
    updateUniformBuffer();
    uint32_t imageIndex;
    auto result = m_SwapChain->acquireNextImage(&imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
    }
    result = m_SwapChain->enqueueCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
    if (result != VK_SUCCESS) {throw std::runtime_error("failed to present swap chain image!");}
}

void HelloWorld::createVertexBuffer() 
{
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_Device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory
    );

    void* data;
    vkMapMemory(m_Device->device(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_Device->device(), stagingBufferMemory);

    m_Device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_VertexBuffer,
        m_VertexBufferMemory
    );

    m_Device->copyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

    vkDestroyBuffer(m_Device->device(), stagingBuffer, nullptr);
    vkFreeMemory(m_Device->device(), stagingBufferMemory, nullptr);
}

void HelloWorld::createDescriptorSetLayout() 
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(m_Device->device(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void HelloWorld::createUniformBuffer() 
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    m_Device->createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_UniformBuffer,
        m_UniformBufferMemory
    );
}

void HelloWorld::createDescriptorPoolAndSet()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(m_Device->device(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) 
    {throw std::runtime_error("failed to create descriptor pool!");}

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_DescriptorSetLayout;
    
    if (vkAllocateDescriptorSets(m_Device->device(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) 
    {throw std::runtime_error("failed to allocate descriptor set!");}

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_UniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_DescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    vkUpdateDescriptorSets(m_Device->device(), 1, &descriptorWrite, 0, nullptr);
}

void HelloWorld::updateUniformBuffer() {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain->swapChainExtent().width / (float)m_SwapChain->swapChainExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    vkMapMemory(m_Device->device(), m_UniformBufferMemory, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_Device->device(), m_UniformBufferMemory);
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