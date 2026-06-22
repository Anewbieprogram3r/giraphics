#include <Engine/Graphics/Vulkan/Pipeline.hpp>

#include <Engine/Graphics/Utils/Utils.hpp>

namespace giraphics {

    Pipeline::Pipeline(Device& device)
        : m_Device{ device } {
    }

    Pipeline::~Pipeline() {
        vkDestroyPipeline(m_Device.device(), m_GraphicsPipeline, nullptr);
    }

    void Pipeline::createGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const PipelineParameters& parameters) {
        ASSERT_WITH_MSG(parameters.pipelineLayout != VK_NULL_HANDLE, "pipelineLayout not specified in PipelineParameters parameters.");
        ASSERT_WITH_MSG(parameters.renderPass != VK_NULL_HANDLE, "renderPass not specified in PipelineParameters parameters.");

        auto vertexShaderSrc = readFile(vertexShaderPath);
        auto fragmentShaderSrc = readFile(fragmentShaderPath);

        auto createShaderModule = [this](const std::vector<char>& code) -> VkShaderModule {
            VkShaderModuleCreateInfo shaderModuleCreateInfo{};
            shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderModuleCreateInfo.pNext = nullptr;
            shaderModuleCreateInfo.flags = 0;
            shaderModuleCreateInfo.codeSize = code.size();
            shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(m_Device.device(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create shader module");
            }

            return shaderModule;
            };

        VkShaderModule vertexShaderModule = createShaderModule(vertexShaderSrc);
        VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderSrc);

        VkPipelineShaderStageCreateInfo shaderStages[2];

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertexShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragmentShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto& bindingDescriptions = parameters.bindingDescriptions;
        auto& attributeDescriptions = parameters.attributeDescriptions;

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &parameters.inputAssemblyCI,
            .pViewportState = &parameters.viewportCI,
            .pRasterizationState = &parameters.rasterizationCI,
            .pMultisampleState = &parameters.multisampleCI,
            .pDepthStencilState = &parameters.depthStencilCI,
            .pColorBlendState = &parameters.colorBlendCI,
            .pDynamicState = &parameters.dynamicStateCI,
            .layout = parameters.pipelineLayout,
            .renderPass = parameters.renderPass,
            .subpass = parameters.subpassIndex,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        if (vkCreateGraphicsPipelines(
            m_Device.device(),
            VK_NULL_HANDLE,
            1,
            &pipelineInfo,
            nullptr,
            &m_GraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline");
        }

        vkDestroyShaderModule(m_Device.device(), vertexShaderModule, nullptr);
        vkDestroyShaderModule(m_Device.device(), fragmentShaderModule, nullptr);
    }

    void Pipeline::bind(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
    }

}  // namespace giraphics