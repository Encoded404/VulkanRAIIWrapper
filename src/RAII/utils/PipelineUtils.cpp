#include "PipelineUtils.hpp"
#include <cstdint>
#include <vector>

namespace VulkanEngine::RAII::Utils {

VkViewport PipelineUtils::create_viewport(float width,
                                         float height,
                                         float min_depth,
                                         float max_depth,
                                         float x,
                                         float y) {
    VkViewport viewport{};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = min_depth;
    viewport.maxDepth = max_depth;
    return viewport;
}

VkRect2D PipelineUtils::create_scissor(uint32_t width, uint32_t height, int32_t offset_x, int32_t offset_y) {
    VkRect2D scissor{};
    scissor.offset = {offset_x, offset_y};
    scissor.extent = {width, height};
    return scissor;
}

VkPipelineColorBlendAttachmentState PipelineUtils::create_default_color_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment{};
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT |
                                VK_COLOR_COMPONENT_A_BIT;
    attachment.blendEnable = VK_FALSE;
    return attachment;
}

VkPipelineColorBlendAttachmentState PipelineUtils::create_alpha_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment = create_default_color_blend_attachment();
    attachment.blendEnable = VK_TRUE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return attachment;
}

VkPipelineColorBlendAttachmentState PipelineUtils::create_additive_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment = create_default_color_blend_attachment();
    attachment.blendEnable = VK_TRUE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return attachment;
}

VkPipelineColorBlendAttachmentState PipelineUtils::create_no_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment = create_default_color_blend_attachment();
    attachment.colorWriteMask = 0;
    return attachment;
}

VkPipelineDepthStencilStateCreateInfo PipelineUtils::create_default_depth_stencil_state() {
    VkPipelineDepthStencilStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = VK_TRUE;
    info.depthCompareOp = VK_COMPARE_OP_LESS;
    info.depthBoundsTestEnable = VK_FALSE;
    info.stencilTestEnable = VK_FALSE;
    info.minDepthBounds = 0.0f;
    info.maxDepthBounds = 1.0f;
    return info;
}

VkPipelineDepthStencilStateCreateInfo PipelineUtils::create_depth_test_only_state() {
    VkPipelineDepthStencilStateCreateInfo info = create_default_depth_stencil_state();
    info.depthWriteEnable = VK_FALSE;
    return info;
}

VkPipelineDepthStencilStateCreateInfo PipelineUtils::create_no_depth_test_state() {
    VkPipelineDepthStencilStateCreateInfo info = create_default_depth_stencil_state();
    info.depthTestEnable = VK_FALSE;
    info.depthWriteEnable = VK_FALSE;
    return info;
}

VkPipelineRasterizationStateCreateInfo PipelineUtils::create_default_rasterization_state() {
    VkPipelineRasterizationStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    info.depthClampEnable = VK_FALSE;
    info.rasterizerDiscardEnable = VK_FALSE;
    info.polygonMode = VK_POLYGON_MODE_FILL;
    info.cullMode = VK_CULL_MODE_BACK_BIT;
    info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    info.depthBiasEnable = VK_FALSE;
    info.lineWidth = 1.0f;
    return info;
}

VkPipelineRasterizationStateCreateInfo PipelineUtils::create_wireframe_rasterization_state() {
    VkPipelineRasterizationStateCreateInfo info = create_default_rasterization_state();
    info.polygonMode = VK_POLYGON_MODE_LINE;
    return info;
}

VkPipelineRasterizationStateCreateInfo PipelineUtils::create_no_cull_rasterization_state() {
    VkPipelineRasterizationStateCreateInfo info = create_default_rasterization_state();
    info.cullMode = VK_CULL_MODE_NONE;
    return info;
}

VkPipelineMultisampleStateCreateInfo PipelineUtils::create_default_multisample_state() {
    VkPipelineMultisampleStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.sampleShadingEnable = VK_FALSE;
    return info;
}

VkPipelineMultisampleStateCreateInfo PipelineUtils::create_msaa_multisample_state(VkSampleCountFlagBits samples) {
    VkPipelineMultisampleStateCreateInfo info = create_default_multisample_state();
    info.rasterizationSamples = samples;
    return info;
}

VkVertexInputBindingDescription PipelineUtils::create_vertex_input_binding(uint32_t binding,
                                                                        uint32_t stride,
                                                                        VkVertexInputRate input_rate) {
    VkVertexInputBindingDescription desc{};
    desc.binding = binding;
    desc.stride = stride;
    desc.inputRate = input_rate;
    return desc;
}

VkVertexInputAttributeDescription PipelineUtils::create_vertex_input_attribute(uint32_t location,
                                                                            uint32_t binding,
                                                                            VkFormat format,
                                                                            uint32_t offset) {
    VkVertexInputAttributeDescription desc{};
    desc.location = location;
    desc.binding = binding;
    desc.format = format;
    desc.offset = offset;
    return desc;
}

VkPipelineShaderStageCreateInfo PipelineUtils::create_shader_stage_create_info(VkShaderStageFlagBits stage,
                                                                           VkShaderModule module,
                                                                           const char* entry_point,
                                                                           const VkSpecializationInfo* specialization_info) {
    VkPipelineShaderStageCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    info.stage = stage;
    info.module = module;
    info.pName = entry_point ? entry_point : "main";
    info.pSpecializationInfo = specialization_info;
    return info;
}

VkPipelineInputAssemblyStateCreateInfo PipelineUtils::create_input_assembly_state(VkPrimitiveTopology topology,
                                                                               VkBool32 primitive_restart_enable) {
    VkPipelineInputAssemblyStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    info.topology = topology;
    info.primitiveRestartEnable = primitive_restart_enable;
    return info;
}

VkPipelineTessellationStateCreateInfo PipelineUtils::create_tessellation_state(uint32_t patch_control_points) {
    VkPipelineTessellationStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
    info.patchControlPoints = patch_control_points;
    return info;
}

VkPipelineViewportStateCreateInfo PipelineUtils::create_viewport_state(const std::vector<VkViewport>& viewports,
                                                                     const std::vector<VkRect2D>& scissors) {
    VkPipelineViewportStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    info.viewportCount = static_cast<uint32_t>(viewports.size());
    info.pViewports = viewports.empty() ? nullptr : viewports.data();
    info.scissorCount = static_cast<uint32_t>(scissors.size());
    info.pScissors = scissors.empty() ? nullptr : scissors.data();
    return info;
}

VkPipelineViewportStateCreateInfo PipelineUtils::create_dynamic_viewport_state(uint32_t viewport_count,
                                                                            uint32_t scissor_count) {
    VkPipelineViewportStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    info.viewportCount = viewport_count;
    info.pViewports = nullptr;
    info.scissorCount = scissor_count;
    info.pScissors = nullptr;
    return info;
}

VkPipelineColorBlendStateCreateInfo PipelineUtils::create_color_blend_state(const std::vector<VkPipelineColorBlendAttachmentState>& attachments,
                                                                         VkBool32 logic_op_enable,
                                                                         VkLogicOp logic_op) {
    VkPipelineColorBlendStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    info.logicOpEnable = logic_op_enable;
    info.logicOp = logic_op;
    info.attachmentCount = static_cast<uint32_t>(attachments.size());
    info.pAttachments = attachments.empty() ? nullptr : attachments.data();
    return info;
}

VkPipelineDynamicStateCreateInfo PipelineUtils::create_dynamic_state(const std::vector<VkDynamicState>& dynamic_states) {
    VkPipelineDynamicStateCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    info.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    info.pDynamicStates = dynamic_states.empty() ? nullptr : dynamic_states.data();
    return info;
}

std::vector<VkDynamicState> PipelineUtils::get_basic_dynamic_states() {
    return {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
}

std::vector<VkDynamicState> PipelineUtils::get_extended_dynamic_states() {
    return {VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
            VK_DYNAMIC_STATE_DEPTH_BIAS,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS};
}

} // namespace VulkanEngine::RAII::Utils


