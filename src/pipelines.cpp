#include <fstream>
#include <vulkan/vulkan_core.h>

#include "vulkan-objects.hpp"

using pooper_cube::shader_module_t;
using pooper_cube::graphics_pipeline_t;
using pooper_cube::pipeline_layout_t;

shader_module_t::shader_module_t(const device_t& p_device, type_t p_type, std::string_view p_code_path) : m_device(p_device) {
    std::ifstream file{p_code_path.data(), std::ios::ate | std::ios::binary};
    if (!file) {
        throw file_opening_exception_t{p_code_path};
    }

    const size_t file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0).read(buffer.data(), file_size);

    file.close();

    const VkShaderModuleCreateInfo module_info {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = file_size,
        .pCode = reinterpret_cast<const uint32_t*>(buffer.data()),
    };

    const auto result = vkCreateShaderModule(m_device, &module_info, nullptr, &m_module);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "shader module"};
    }
}

pipeline_layout_t::pipeline_layout_t(
        const device_t& p_device, 
        std::span<const VkDescriptorSetLayout> p_set_layouts, 
        std::span<const VkPushConstantRange> p_push_constant_ranges
) : m_device(p_device) {
    const VkPipelineLayoutCreateInfo layout_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32_t>(p_set_layouts.size()),
        .pSetLayouts = p_set_layouts.data(),
        .pushConstantRangeCount = static_cast<uint32_t>(p_push_constant_ranges.size()),
        .pPushConstantRanges = p_push_constant_ranges.data(),
    };

    const auto result = vkCreatePipelineLayout(m_device, &layout_info, nullptr, &m_layout);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "pipeline layout"};
    }
}

graphics_pipeline_t::graphics_pipeline_t(
        const device_t& p_device, 
        const shader_module_t& p_vertex_module, 
        const shader_module_t& p_fragment_module,
        const pipeline_layout_t& p_layout
) : m_device(p_device) {
    const std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages {
        p_vertex_module.get_shader_stage(),
        p_fragment_module.get_shader_stage()
    };

    const VkVertexInputBindingDescription vertex_binding_description {
        .binding = 0,
        .stride = sizeof(vertex_t),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    const VkPipelineVertexInputStateCreateInfo vertex_input_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_binding_description,
        .vertexAttributeDescriptionCount = vertex_attribute_descriptions.size(),
        .pVertexAttributeDescriptions = vertex_attribute_descriptions.data(),
    };

    const VkPipelineInputAssemblyStateCreateInfo input_assembly_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    const VkPipelineViewportStateCreateInfo viewport_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        // These can be null, as they are specified as dynamic states.
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    const VkPipelineRasterizationStateCreateInfo rasterizer_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };
    
    const VkPipelineMultisampleStateCreateInfo multisampling_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    const VkPipelineColorBlendAttachmentState color_blend_attachment {
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT,
    };

    const VkPipelineColorBlendStateCreateInfo color_blend_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_NO_OP,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {
            0.0f, 0.0f, 0.0f, 0.0f
        }
    };

    const std::array<VkDynamicState, 2> dynamic_states {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    const VkPipelineDynamicStateCreateInfo dynamic_state {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = dynamic_states.size(),
        .pDynamicStates = dynamic_states.data(),
    };

    const VkGraphicsPipelineCreateInfo pipeline_info {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = shader_stages.size(),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer_state,
        .pMultisampleState = &multisampling_state,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = &dynamic_state,
        .layout = p_layout,
        .renderPass = VK_NULL_HANDLE,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    const auto result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_pipeline);
    if (result != VK_SUCCESS) {
        throw vulkan_creation_exception_t{result, "graphics pipeline"};
    }
}
