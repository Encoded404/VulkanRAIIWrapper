#include "Pipeline.hpp"

#include "RenderPass.hpp"
#include "../core/Device.hpp"
#include "rendering/PipelineStructs.hpp"

#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

namespace {
std::vector<VkPipelineShaderStageCreateInfo> BuildShaderStages(const std::vector<PipelineShaderStage>& stages) {
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    shader_stages.reserve(stages.size());
    for (const auto& stage : stages) {
        if (stage.module == VK_NULL_HANDLE) {
            throw std::invalid_argument("Shader stage requires a valid shader module");
        }

        VkPipelineShaderStageCreateInfo stage_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stage_info.stage = stage.stage;
        stage_info.module = stage.module;
        stage_info.pName = stage.entryPoint.empty() ? "main" : stage.entryPoint.c_str();
        stage_info.pSpecializationInfo = stage.specializationInfo;
        shader_stages.push_back(stage_info);
    }
    return shader_stages;
}

VkPipelineVertexInputStateCreateInfo BuildVertexInputState(const PipelineVertexInput& vertex_input,
                                                           std::vector<VkVertexInputBindingDescription>& binding_descriptions,
                                                           std::vector<VkVertexInputAttributeDescription>& attribute_descriptions) {
    binding_descriptions = vertex_input.bindingDescriptions;
    attribute_descriptions = vertex_input.attributeDescriptions;

    VkPipelineVertexInputStateCreateInfo vertex_input_info{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.empty() ? nullptr : binding_descriptions.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.empty() ? nullptr : attribute_descriptions.data();
    return vertex_input_info;
}

VkPipelineInputAssemblyStateCreateInfo BuildInputAssemblyState(const PipelineInputAssembly& input_assembly) {
    VkPipelineInputAssemblyStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    info.topology = input_assembly.topology;
    info.primitiveRestartEnable = input_assembly.primitiveRestartEnable;
    return info;
}

VkPipelineViewportStateCreateInfo BuildViewportState(const PipelineViewport& viewport_state,
                                                      std::vector<VkViewport>& viewports,
                                                      std::vector<VkRect2D>& scissors) {
    viewports = viewport_state.viewports;
    scissors = viewport_state.scissors;

    VkPipelineViewportStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    info.viewportCount = static_cast<uint32_t>(viewports.size());
    info.pViewports = viewports.empty() ? nullptr : viewports.data();
    info.scissorCount = static_cast<uint32_t>(scissors.size());
    info.pScissors = scissors.empty() ? nullptr : scissors.data();
    return info;
}

VkPipelineRasterizationStateCreateInfo BuildRasterizationState(const PipelineRasterization& rasterization) {
    VkPipelineRasterizationStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    info.depthClampEnable = rasterization.depthClampEnable;
    info.rasterizerDiscardEnable = rasterization.rasterizerDiscardEnable;
    info.polygonMode = rasterization.polygonMode;
    info.cullMode = rasterization.cullMode;
    info.frontFace = rasterization.frontFace;
    info.depthBiasEnable = rasterization.depthBiasEnable;
    info.depthBiasConstantFactor = rasterization.depthBiasConstantFactor;
    info.depthBiasClamp = rasterization.depthBiasClamp;
    info.depthBiasSlopeFactor = rasterization.depthBiasSlopeFactor;
    info.lineWidth = rasterization.lineWidth;
    return info;
}

VkPipelineMultisampleStateCreateInfo BuildMultisampleState(const PipelineMultisample& multisample) {
    VkPipelineMultisampleStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    info.rasterizationSamples = multisample.rasterizationSamples;
    info.sampleShadingEnable = multisample.sampleShadingEnable;
    info.minSampleShading = multisample.minSampleShading;
    info.pSampleMask = multisample.sampleMask;
    info.alphaToCoverageEnable = multisample.alphaToCoverageEnable;
    info.alphaToOneEnable = multisample.alphaToOneEnable;
    return info;
}

VkPipelineDepthStencilStateCreateInfo BuildDepthStencilState(const PipelineDepthStencil& depth_stencil) {
    VkPipelineDepthStencilStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    info.depthTestEnable = depth_stencil.depthTestEnable;
    info.depthWriteEnable = depth_stencil.depthWriteEnable;
    info.depthCompareOp = depth_stencil.depthCompareOp;
    info.depthBoundsTestEnable = depth_stencil.depthBoundsTestEnable;
    info.stencilTestEnable = depth_stencil.stencilTestEnable;
    info.front = depth_stencil.front;
    info.back = depth_stencil.back;
    info.minDepthBounds = depth_stencil.minDepthBounds;
    info.maxDepthBounds = depth_stencil.maxDepthBounds;
    return info;
}

VkPipelineColorBlendStateCreateInfo BuildColorBlendState(const PipelineColorBlend& color_blend,
                                                          std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
    attachments = color_blend.attachments;

    VkPipelineColorBlendStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    info.logicOpEnable = color_blend.logicOpEnable;
    info.logicOp = color_blend.logicOp;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.empty() ? nullptr : attachments.data();
    info.blendConstants[0] = color_blend.blendConstants[0];
    info.blendConstants[1] = color_blend.blendConstants[1];
    info.blendConstants[2] = color_blend.blendConstants[2];
    info.blendConstants[3] = color_blend.blendConstants[3];
    return info;
}

VkPipelineDynamicStateCreateInfo BuildDynamicState(const std::vector<VkDynamicState>& dynamic_states,
                                                    std::vector<VkDynamicState>& state_storage) {
    state_storage = dynamic_states;
    VkPipelineDynamicStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    info.dynamicStateCount = static_cast<uint32_t>(state_storage.size());
    info.pDynamicStates = state_storage.empty() ? nullptr : state_storage.data();
    return info;
}
}

Pipeline::Pipeline(const Device& device,
                   const RenderPass& render_pass,
                   VkPipelineLayout layout,
                   const std::vector<PipelineShaderStage>& shader_stages,
                   const PipelineVertexInput& vertex_input,
                   const PipelineInputAssembly& input_assembly,
                   const PipelineViewport& viewport,
                   const PipelineRasterization& rasterization,
                   const PipelineMultisample& multisample,
                   const PipelineDepthStencil& depth_stencil,
                   const PipelineColorBlend& color_blend,
                   const std::vector<VkDynamicState>& dynamic_states,
                   uint32_t subpass,
                   VkPipeline base_pipeline,
                   int32_t base_pipeline_index)
    : device_(device.GetHandle()),
      layout_(layout),
      type_(Type::GRAPHICS) {
    if (device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE) {
        throw std::invalid_argument("Pipeline requires valid device and layout");
    }
    CreateGraphicsPipeline(render_pass,
                           shader_stages,
                           vertex_input,
                           input_assembly,
                           viewport,
                           rasterization,
                           multisample,
                           depth_stencil,
                           color_blend,
                           dynamic_states,
                           subpass,
                           base_pipeline,
                           base_pipeline_index);
}

Pipeline::Pipeline(const Device& device,
                   const RenderPass& render_pass,
                   VkPipelineLayout layout,
                   const std::vector<PipelineShaderStage>& shader_stages,
                   const PipelineVertexInput& vertex_input,
                   const PipelineInputAssembly& input_assembly,
                   const PipelineTessellation& tessellation,
                   const PipelineViewport& viewport,
                   const PipelineRasterization& rasterization,
                   const PipelineMultisample& multisample,
                   const PipelineDepthStencil& depth_stencil,
                   const PipelineColorBlend& color_blend,
                   const std::vector<VkDynamicState>& dynamic_states,
                   uint32_t subpass,
                   VkPipeline base_pipeline,
                   int32_t base_pipeline_index)
    : device_(device.GetHandle()),
      layout_(layout),
      type_(Type::GRAPHICS) {
    if (device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE) {
        throw std::invalid_argument("Pipeline requires valid device and layout");
    }
    CreateGraphicsPipelineWithTessellation(render_pass,
                                           shader_stages,
                                           vertex_input,
                                           input_assembly,
                                           tessellation,
                                           viewport,
                                           rasterization,
                                           multisample,
                                           depth_stencil,
                                           color_blend,
                                           dynamic_states,
                                           subpass,
                                           base_pipeline,
                                           base_pipeline_index);
}

Pipeline::Pipeline(const Device& device,
                   VkPipelineLayout layout,
                   const PipelineShaderStage& compute_stage,
                   VkPipeline base_pipeline,
                   int32_t base_pipeline_index)
    : device_(device.GetHandle()),
      layout_(layout),
      type_(Type::COMPUTE) {
    if (device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE) {
        throw std::invalid_argument("Pipeline requires valid device and layout");
    }
    CreateComputePipeline(compute_stage, base_pipeline, base_pipeline_index);
}

Pipeline::~Pipeline() {
    Cleanup();
}

Pipeline::Pipeline(Pipeline&& other) noexcept
    : pipeline_(other.pipeline_),
      device_(other.device_),
      layout_(other.layout_),
      type_(other.type_) {
    other.pipeline_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.layout_ = VK_NULL_HANDLE;
    other.type_ = Type::GRAPHICS;
}

Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
    if (this != &other) {
        Cleanup();
        pipeline_ = other.pipeline_;
        device_ = other.device_;
        layout_ = other.layout_;
        type_ = other.type_;
        other.pipeline_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.layout_ = VK_NULL_HANDLE;
        other.type_ = Type::GRAPHICS;
    }
    return *this;
}

void Pipeline::CreateGraphicsPipeline(const RenderPass& render_pass,
                                      const std::vector<PipelineShaderStage>& shader_stages,
                                      const PipelineVertexInput& vertex_input,
                                      const PipelineInputAssembly& input_assembly,
                                      const PipelineViewport& viewport,
                                      const PipelineRasterization& rasterization,
                                      const PipelineMultisample& multisample,
                                      const PipelineDepthStencil& depth_stencil,
                                      const PipelineColorBlend& color_blend,
                                      const std::vector<VkDynamicState>& dynamic_states,
                                      uint32_t subpass,
                                      VkPipeline base_pipeline,
                                      int32_t base_pipeline_index) {
    auto vk_shader_stages = BuildShaderStages(shader_stages);

    std::vector<VkVertexInputBindingDescription> binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    VkPipelineVertexInputStateCreateInfo vertex_input_info = BuildVertexInputState(vertex_input,
                                                                                 binding_descriptions,
                                                                                 attribute_descriptions);

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = BuildInputAssemblyState(input_assembly);

    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    VkPipelineViewportStateCreateInfo viewport_info = BuildViewportState(viewport, viewports, scissors);

    VkPipelineRasterizationStateCreateInfo rasterization_info = BuildRasterizationState(rasterization);
    VkPipelineMultisampleStateCreateInfo multisample_info = BuildMultisampleState(multisample);
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = BuildDepthStencilState(depth_stencil);

    std::vector<VkPipelineColorBlendAttachmentState> color_attachments;
    VkPipelineColorBlendStateCreateInfo color_blend_info = BuildColorBlendState(color_blend, color_attachments);

    std::vector<VkDynamicState> dynamic_state_storage;
    VkPipelineDynamicStateCreateInfo dynamic_state_info = BuildDynamicState(dynamic_states, dynamic_state_storage);

    VkGraphicsPipelineCreateInfo pipeline_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeline_info.stageCount = static_cast<uint32_t>(vk_shader_stages.size());
    pipeline_info.pStages = vk_shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pViewportState = &viewport_info;
    pipeline_info.pRasterizationState = &rasterization_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pDepthStencilState = depth_stencil_info.depthTestEnable || depth_stencil_info.stencilTestEnable ? &depth_stencil_info : nullptr;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = dynamic_state_storage.empty() ? nullptr : &dynamic_state_info;
    pipeline_info.layout = layout_;
    pipeline_info.renderPass = render_pass.GetHandle();
    pipeline_info.subpass = subpass;
    pipeline_info.basePipelineHandle = base_pipeline;
    pipeline_info.basePipelineIndex = base_pipeline_index;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
}

void Pipeline::CreateGraphicsPipelineWithTessellation(const RenderPass& render_pass,
                                                      const std::vector<PipelineShaderStage>& shader_stages,
                                                      const PipelineVertexInput& vertex_input,
                                                      const PipelineInputAssembly& input_assembly,
                                                      const PipelineTessellation& tessellation,
                                                      const PipelineViewport& viewport,
                                                      const PipelineRasterization& rasterization,
                                                      const PipelineMultisample& multisample,
                                                      const PipelineDepthStencil& depth_stencil,
                                                      const PipelineColorBlend& color_blend,
                                                      const std::vector<VkDynamicState>& dynamic_states,
                                                      uint32_t subpass,
                                                      VkPipeline base_pipeline,
                                                      int32_t base_pipeline_index) {
    auto vk_shader_stages = BuildShaderStages(shader_stages);

    std::vector<VkVertexInputBindingDescription> binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    VkPipelineVertexInputStateCreateInfo vertex_input_info = BuildVertexInputState(vertex_input,
                                                                                 binding_descriptions,
                                                                                 attribute_descriptions);

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = BuildInputAssemblyState(input_assembly);

    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    VkPipelineViewportStateCreateInfo viewport_info = BuildViewportState(viewport, viewports, scissors);

    VkPipelineRasterizationStateCreateInfo rasterization_info = BuildRasterizationState(rasterization);
    VkPipelineMultisampleStateCreateInfo multisample_info = BuildMultisampleState(multisample);
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = BuildDepthStencilState(depth_stencil);

    std::vector<VkPipelineColorBlendAttachmentState> color_attachments;
    VkPipelineColorBlendStateCreateInfo color_blend_info = BuildColorBlendState(color_blend, color_attachments);

    std::vector<VkDynamicState> dynamic_state_storage;
    VkPipelineDynamicStateCreateInfo dynamic_state_info = BuildDynamicState(dynamic_states, dynamic_state_storage);

    VkPipelineTessellationStateCreateInfo tessellation_info{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
    tessellation_info.patchControlPoints = tessellation.patchControlPoints;

    VkGraphicsPipelineCreateInfo pipeline_info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeline_info.stageCount = static_cast<uint32_t>(vk_shader_stages.size());
    pipeline_info.pStages = vk_shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly_info;
    pipeline_info.pTessellationState = &tessellation_info;
    pipeline_info.pViewportState = &viewport_info;
    pipeline_info.pRasterizationState = &rasterization_info;
    pipeline_info.pMultisampleState = &multisample_info;
    pipeline_info.pDepthStencilState = depth_stencil_info.depthTestEnable || depth_stencil_info.stencilTestEnable ? &depth_stencil_info : nullptr;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = dynamic_state_storage.empty() ? nullptr : &dynamic_state_info;
    pipeline_info.layout = layout_;
    pipeline_info.renderPass = render_pass.GetHandle();
    pipeline_info.subpass = subpass;
    pipeline_info.basePipelineHandle = base_pipeline;
    pipeline_info.basePipelineIndex = base_pipeline_index;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create tessellated graphics pipeline");
    }
}

void Pipeline::CreateComputePipeline(const PipelineShaderStage& compute_stage,
                                     VkPipeline base_pipeline,
                                     int32_t base_pipeline_index) {
    if (compute_stage.module == VK_NULL_HANDLE || compute_stage.stage != VK_SHADER_STAGE_COMPUTE_BIT) {
        throw std::invalid_argument("Compute pipeline requires a compute shader stage");
    }

    VkPipelineShaderStageCreateInfo stage_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stage_info.stage = compute_stage.stage;
    stage_info.module = compute_stage.module;
    stage_info.pName = compute_stage.entryPoint.empty() ? "main" : compute_stage.entryPoint.c_str();
    stage_info.pSpecializationInfo = compute_stage.specializationInfo;

    VkComputePipelineCreateInfo pipeline_info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipeline_info.stage = stage_info;
    pipeline_info.layout = layout_;
    pipeline_info.basePipelineHandle = base_pipeline;
    pipeline_info.basePipelineIndex = base_pipeline_index;

    if (vkCreateComputePipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline");
    }
}

void Pipeline::Cleanup() {
    if (pipeline_ != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_, pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

