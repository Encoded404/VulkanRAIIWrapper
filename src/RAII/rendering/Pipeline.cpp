#include "Pipeline.hpp"

#include "RenderPass.hpp"
#include "../core/Device.hpp"
#include "rendering/PipelineStructs.hpp"

#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

namespace {
std::vector<VkPipelineShaderStageCreateInfo> build_shader_stages(const std::vector<PipelineShaderStage>& stages) {
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
    shader_stages.reserve(stages.size());
    for (const auto& stage : stages) {
        if (stage.module_ == VK_NULL_HANDLE) {
            throw std::invalid_argument("Shader stage requires a valid shader module");
        }

        VkPipelineShaderStageCreateInfo stage_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        stage_info.stage = stage.stage_;
        stage_info.module = stage.module_;
        stage_info.pName = stage.entryPoint_.empty() ? "main" : stage.entryPoint_.c_str();
        stage_info.pSpecializationInfo = stage.specializationInfo_;
        shader_stages.push_back(stage_info);
    }
    return shader_stages;
}

VkPipelineVertexInputStateCreateInfo build_vertex_input_state(const PipelineVertexInput& vertex_input,
                                                           std::vector<VkVertexInputBindingDescription>& binding_descriptions,
                                                           std::vector<VkVertexInputAttributeDescription>& attribute_descriptions) {
    binding_descriptions = vertex_input.bindingDescriptions_;
    attribute_descriptions = vertex_input.attributeDescriptions_;

    VkPipelineVertexInputStateCreateInfo vertex_input_info{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.empty() ? nullptr : binding_descriptions.data();
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.empty() ? nullptr : attribute_descriptions.data();
    return vertex_input_info;
}

VkPipelineInputAssemblyStateCreateInfo build_input_assembly_state(const PipelineInputAssembly& input_assembly) {
    VkPipelineInputAssemblyStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    info.topology = input_assembly.topology_;
    info.primitiveRestartEnable = input_assembly.primitiveRestartEnable_;
    return info;
}

VkPipelineViewportStateCreateInfo build_viewport_state(const PipelineViewport& viewport_state,
                                                      std::vector<VkViewport>& viewports,
                                                      std::vector<VkRect2D>& scissors) {
    viewports = viewport_state.viewports_;
    scissors = viewport_state.scissors_;

    VkPipelineViewportStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    info.viewportCount = static_cast<uint32_t>(viewports.size());
    info.pViewports = viewports.empty() ? nullptr : viewports.data();
    info.scissorCount = static_cast<uint32_t>(scissors.size());
    info.pScissors = scissors.empty() ? nullptr : scissors.data();
    return info;
}

VkPipelineRasterizationStateCreateInfo build_rasterization_state(const PipelineRasterization& rasterization) {
    VkPipelineRasterizationStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    info.depthClampEnable = rasterization.depthClampEnable_;
    info.rasterizerDiscardEnable = rasterization.rasterizerDiscardEnable_;
    info.polygonMode = rasterization.polygonMode_;
    info.cullMode = rasterization.cullMode_;
    info.frontFace = rasterization.frontFace_;
    info.depthBiasEnable = rasterization.depthBiasEnable_;
    info.depthBiasConstantFactor = rasterization.depthBiasConstantFactor_;
    info.depthBiasClamp = rasterization.depthBiasClamp_;
    info.depthBiasSlopeFactor = rasterization.depthBiasSlopeFactor_;
    info.lineWidth = rasterization.lineWidth_;
    return info;
}

VkPipelineMultisampleStateCreateInfo build_multisample_state(const PipelineMultisample& multisample) {
    VkPipelineMultisampleStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    info.rasterizationSamples = multisample.rasterizationSamples_;
    info.sampleShadingEnable = multisample.sampleShadingEnable_;
    info.minSampleShading = multisample.minSampleShading_;
    info.pSampleMask = multisample.sampleMask_;
    info.alphaToCoverageEnable = multisample.alphaToCoverageEnable_;
    info.alphaToOneEnable = multisample.alphaToOneEnable_;
    return info;
}

VkPipelineDepthStencilStateCreateInfo build_depth_stencil_state(const PipelineDepthStencil& depth_stencil) {
    VkPipelineDepthStencilStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    info.depthTestEnable = depth_stencil.depthTestEnable_;
    info.depthWriteEnable = depth_stencil.depthWriteEnable_;
    info.depthCompareOp = depth_stencil.depthCompareOp_;
    info.depthBoundsTestEnable = depth_stencil.depthBoundsTestEnable_;
    info.stencilTestEnable = depth_stencil.stencilTestEnable_;
    info.front = depth_stencil.front_;
    info.back = depth_stencil.back_;
    info.minDepthBounds = depth_stencil.minDepthBounds_;
    info.maxDepthBounds = depth_stencil.maxDepthBounds_;
    return info;
}

VkPipelineColorBlendStateCreateInfo build_color_blend_state(const PipelineColorBlend& color_blend,
                                                          std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
    attachments = color_blend.attachments_;

    VkPipelineColorBlendStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    info.logicOpEnable = color_blend.logicOpEnable_;
    info.logicOp = color_blend.logicOp_;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.empty() ? nullptr : attachments.data();
    info.blendConstants[0] = color_blend.blendConstants_[0];
    info.blendConstants[1] = color_blend.blendConstants_[1];
    info.blendConstants[2] = color_blend.blendConstants_[2];
    info.blendConstants[3] = color_blend.blendConstants_[3];
    return info;
}

VkPipelineDynamicStateCreateInfo build_dynamic_state(const std::vector<VkDynamicState>& dynamic_states,
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
    : device_(device.get_handle()),
      layout_(layout),
      type_(Type::GRAPHICS) {
    if (device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE) {
        throw std::invalid_argument("Pipeline requires valid device and layout");
    }
    create_graphics_pipeline(render_pass,
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
    : device_(device.get_handle()),
      layout_(layout),
      type_(Type::GRAPHICS) {
    if (device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE) {
        throw std::invalid_argument("Pipeline requires valid device and layout");
    }
    create_graphics_pipeline_with_tessellation(render_pass,
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
    : device_(device.get_handle()),
      layout_(layout),
      type_(Type::COMPUTE) {
    if (device == VK_NULL_HANDLE || layout == VK_NULL_HANDLE) {
        throw std::invalid_argument("Pipeline requires valid device and layout");
    }
    create_compute_pipeline(compute_stage, base_pipeline, base_pipeline_index);
}

Pipeline::~Pipeline() {
    cleanup();
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
        cleanup();
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

void Pipeline::create_graphics_pipeline(const RenderPass& render_pass,
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
    auto vk_shader_stages = build_shader_stages(shader_stages);

    std::vector<VkVertexInputBindingDescription> binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    VkPipelineVertexInputStateCreateInfo vertex_input_info = build_vertex_input_state(vertex_input,
                                                                                 binding_descriptions,
                                                                                 attribute_descriptions);

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = build_input_assembly_state(input_assembly);

    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    VkPipelineViewportStateCreateInfo viewport_info = build_viewport_state(viewport, viewports, scissors);

    VkPipelineRasterizationStateCreateInfo rasterization_info = build_rasterization_state(rasterization);
    VkPipelineMultisampleStateCreateInfo multisample_info = build_multisample_state(multisample);
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = build_depth_stencil_state(depth_stencil);

    std::vector<VkPipelineColorBlendAttachmentState> color_attachments;
    VkPipelineColorBlendStateCreateInfo color_blend_info = build_color_blend_state(color_blend, color_attachments);

    std::vector<VkDynamicState> dynamic_state_storage;
    VkPipelineDynamicStateCreateInfo dynamic_state_info = build_dynamic_state(dynamic_states, dynamic_state_storage);

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
    pipeline_info.renderPass = render_pass.get_handle();
    pipeline_info.subpass = subpass;
    pipeline_info.basePipelineHandle = base_pipeline;
    pipeline_info.basePipelineIndex = base_pipeline_index;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
}

void Pipeline::create_graphics_pipeline_with_tessellation(const RenderPass& render_pass,
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
    auto vk_shader_stages = build_shader_stages(shader_stages);

    std::vector<VkVertexInputBindingDescription> binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
    VkPipelineVertexInputStateCreateInfo vertex_input_info = build_vertex_input_state(vertex_input,
                                                                                 binding_descriptions,
                                                                                 attribute_descriptions);

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = build_input_assembly_state(input_assembly);

    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
    VkPipelineViewportStateCreateInfo viewport_info = build_viewport_state(viewport, viewports, scissors);

    VkPipelineRasterizationStateCreateInfo rasterization_info = build_rasterization_state(rasterization);
    VkPipelineMultisampleStateCreateInfo multisample_info = build_multisample_state(multisample);
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = build_depth_stencil_state(depth_stencil);

    std::vector<VkPipelineColorBlendAttachmentState> color_attachments;
    VkPipelineColorBlendStateCreateInfo color_blend_info = build_color_blend_state(color_blend, color_attachments);

    std::vector<VkDynamicState> dynamic_state_storage;
    VkPipelineDynamicStateCreateInfo dynamic_state_info = build_dynamic_state(dynamic_states, dynamic_state_storage);

    VkPipelineTessellationStateCreateInfo tessellation_info{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
    tessellation_info.patchControlPoints = tessellation.patchControlPoints_;

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
    pipeline_info.renderPass = render_pass.get_handle();
    pipeline_info.subpass = subpass;
    pipeline_info.basePipelineHandle = base_pipeline;
    pipeline_info.basePipelineIndex = base_pipeline_index;

    if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create tessellated graphics pipeline");
    }
}

void Pipeline::create_compute_pipeline(const PipelineShaderStage& compute_stage,
                                     VkPipeline base_pipeline,
                                     int32_t base_pipeline_index) {
    if (compute_stage.module_ == VK_NULL_HANDLE || compute_stage.stage_ != VK_SHADER_STAGE_COMPUTE_BIT) {
        throw std::invalid_argument("Compute pipeline requires a compute shader stage");
    }

    VkPipelineShaderStageCreateInfo stage_info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stage_info.stage = compute_stage.stage_;
    stage_info.module = compute_stage.module_;
    stage_info.pName = compute_stage.entryPoint_.empty() ? "main" : compute_stage.entryPoint_.c_str();
    stage_info.pSpecializationInfo = compute_stage.specializationInfo_;

    VkComputePipelineCreateInfo pipeline_info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipeline_info.stage = stage_info;
    pipeline_info.layout = layout_;
    pipeline_info.basePipelineHandle = base_pipeline;
    pipeline_info.basePipelineIndex = base_pipeline_index;

    if (vkCreateComputePipelines(device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline");
    }
}

void Pipeline::cleanup() {
    if (pipeline_ != VK_NULL_HANDLE) {
        vkDestroyPipeline(device_, pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

