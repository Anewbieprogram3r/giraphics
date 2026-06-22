#pragma once

#include <Engine/Graphics/Vulkan/Device.hpp>

namespace giraphics {
    struct PipelineParameters;
    class Pipeline : public NonCopyable {
    public:
        Pipeline(Device& device);
        ~Pipeline();

        void createGraphicsPipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const PipelineParameters& parameters);
        void bind(VkCommandBuffer commandBuffer);

    private:
        virtual void createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {};
        virtual void createPipeline(VkRenderPass renderPass) {};

    protected:
        Device& m_Device;
        VkPipeline m_GraphicsPipeline{};
    };

    struct PipelineParameters {
        PipelineParameters(bool initializeConfiguration = true) {
            if (!initializeConfiguration) return;

            inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyCI.primitiveRestartEnable = VK_FALSE;

            viewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportCI.viewportCount = 1;
            viewportCI.pViewports = nullptr;
            viewportCI.scissorCount = 1;
            viewportCI.pScissors = nullptr;

            rasterizationCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationCI.depthClampEnable = VK_FALSE;
            rasterizationCI.rasterizerDiscardEnable = VK_FALSE;
            rasterizationCI.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizationCI.lineWidth = 1.0f;
            rasterizationCI.cullMode = VK_CULL_MODE_NONE;
            rasterizationCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizationCI.depthBiasEnable = VK_FALSE;
            rasterizationCI.depthBiasConstantFactor = 0.0f;
            rasterizationCI.depthBiasClamp = 0.0f;
            rasterizationCI.depthBiasSlopeFactor = 0.0f;

            multisampleCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleCI.sampleShadingEnable = VK_FALSE;
            multisampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampleCI.minSampleShading = 1.0f;
            multisampleCI.pSampleMask = nullptr;
            multisampleCI.alphaToCoverageEnable = VK_FALSE;
            multisampleCI.alphaToOneEnable = VK_FALSE;

            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

            colorBlendCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendCI.logicOpEnable = VK_FALSE;
            colorBlendCI.logicOp = VK_LOGIC_OP_COPY;
            colorBlendCI.attachmentCount = 1;
            colorBlendCI.pAttachments = &colorBlendAttachment;
            colorBlendCI.blendConstants[0] = 0.0f;
            colorBlendCI.blendConstants[1] = 0.0f;
            colorBlendCI.blendConstants[2] = 0.0f;
            colorBlendCI.blendConstants[3] = 0.0f;

            depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilCI.depthTestEnable = VK_TRUE;
            depthStencilCI.depthWriteEnable = VK_TRUE;
            depthStencilCI.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencilCI.depthBoundsTestEnable = VK_FALSE;
            depthStencilCI.minDepthBounds = 0.0f;
            depthStencilCI.maxDepthBounds = 1.0f;
            depthStencilCI.stencilTestEnable = VK_FALSE;
            depthStencilCI.front = {};
            depthStencilCI.back = {};

            dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
            dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
            dynamicStateCI.flags = 0;
        }

        std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        std::vector<VkDynamicState> dynamicStateEnables{};

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlendCI{};
        VkPipelineDepthStencilStateCreateInfo depthStencilCI{};
        VkPipelineDynamicStateCreateInfo dynamicStateCI{};
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
        VkPipelineMultisampleStateCreateInfo multisampleCI{};
        VkPipelineRasterizationStateCreateInfo rasterizationCI{};
        VkPipelineViewportStateCreateInfo viewportCI{};

        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpassIndex = 0;
    };
}  // namespace giraphics