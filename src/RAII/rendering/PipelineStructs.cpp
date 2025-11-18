#include "PipelineStructs.hpp"
#include <cstdint>



namespace VulkanEngine::RAII::PipelineDefaults {

PipelineVertexInput create_empty_vertex_input() {
    return {};
}

PipelineInputAssembly create_triangle_list_input_assembly() {
    PipelineInputAssembly assembly;
    assembly.topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assembly.primitiveRestartEnable_ = VK_FALSE;
    return assembly;
}

PipelineInputAssembly create_triangle_strip_input_assembly() {
    PipelineInputAssembly assembly;
    assembly.topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    assembly.primitiveRestartEnable_ = VK_TRUE;
    return assembly;
}

PipelineInputAssembly create_point_list_input_assembly() {
    PipelineInputAssembly assembly;
    assembly.topology_ = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    assembly.primitiveRestartEnable_ = VK_FALSE;
    return assembly;
}

PipelineInputAssembly create_line_list_input_assembly() {
    PipelineInputAssembly assembly;
    assembly.topology_ = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    assembly.primitiveRestartEnable_ = VK_FALSE;
    return assembly;
}

PipelineRasterization create_default_rasterization() {
    PipelineRasterization rasterization;
    rasterization.depthClampEnable_ = VK_FALSE;
    rasterization.rasterizerDiscardEnable_ = VK_FALSE;
    rasterization.polygonMode_ = VK_POLYGON_MODE_FILL;
    rasterization.cullMode_ = VK_CULL_MODE_BACK_BIT;
    rasterization.frontFace_ = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.depthBiasEnable_ = VK_FALSE;
    rasterization.lineWidth_ = 1.0f;
    return rasterization;
}

PipelineRasterization create_wireframe_rasterization() {
    PipelineRasterization rasterization = create_default_rasterization();
    rasterization.polygonMode_ = VK_POLYGON_MODE_LINE;
    rasterization.lineWidth_ = 1.0f;
    return rasterization;
}

PipelineRasterization create_no_cull_rasterization() {
    PipelineRasterization rasterization = create_default_rasterization();
    rasterization.cullMode_ = VK_CULL_MODE_NONE;
    return rasterization;
}

PipelineMultisample create_no_multisample() {
    PipelineMultisample multisample;
    multisample.rasterizationSamples_ = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable_ = VK_FALSE;
    return multisample;
}

PipelineMultisample create_msa_a4x() {
    PipelineMultisample multisample = create_no_multisample();
    multisample.rasterizationSamples_ = VK_SAMPLE_COUNT_4_BIT;
    return multisample;
}

PipelineMultisample create_msa_a8x() {
    PipelineMultisample multisample = create_no_multisample();
    multisample.rasterizationSamples_ = VK_SAMPLE_COUNT_8_BIT;
    return multisample;
}

PipelineDepthStencil create_default_depth_stencil() {
    PipelineDepthStencil depth_stencil;
    depth_stencil.depthTestEnable_ = VK_TRUE;
    depth_stencil.depthWriteEnable_ = VK_TRUE;
    depth_stencil.depthCompareOp_ = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable_ = VK_FALSE;
    depth_stencil.stencilTestEnable_ = VK_FALSE;
    depth_stencil.minDepthBounds_ = 0.0f;
    depth_stencil.maxDepthBounds_ = 1.0f;
    return depth_stencil;
}

PipelineDepthStencil create_no_depth_test() {
    PipelineDepthStencil depth_stencil = create_default_depth_stencil();
    depth_stencil.depthTestEnable_ = VK_FALSE;
    depth_stencil.depthWriteEnable_ = VK_FALSE;
    return depth_stencil;
}

PipelineDepthStencil create_depth_only_test() {
    PipelineDepthStencil depth_stencil = create_default_depth_stencil();
    depth_stencil.stencilTestEnable_ = VK_FALSE;
    return depth_stencil;
}

VkPipelineColorBlendAttachmentState create_opaque_color_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment{};
    attachment.blendEnable = VK_FALSE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                VK_COLOR_COMPONENT_G_BIT |
                                VK_COLOR_COMPONENT_B_BIT |
                                VK_COLOR_COMPONENT_A_BIT;
    return attachment;
}

VkPipelineColorBlendAttachmentState create_alpha_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment = create_opaque_color_blend_attachment();
    attachment.blendEnable = VK_TRUE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return attachment;
}

VkPipelineColorBlendAttachmentState create_additive_blend_attachment() {
    VkPipelineColorBlendAttachmentState attachment = create_opaque_color_blend_attachment();
    attachment.blendEnable = VK_TRUE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return attachment;
}

PipelineColorBlend create_opaque_color_blend(uint32_t attachment_count) {
    PipelineColorBlend color_blend;
    color_blend.logicOpEnable_ = VK_FALSE;
    color_blend.attachments_.resize(attachment_count, create_opaque_color_blend_attachment());
    return color_blend;
}

PipelineColorBlend create_alpha_blend_color_blend(uint32_t attachment_count) {
    PipelineColorBlend color_blend;
    color_blend.logicOpEnable_ = VK_FALSE;
    color_blend.attachments_.resize(attachment_count, create_alpha_blend_attachment());
    return color_blend;
}

PipelineColorBlend create_additive_blend_color_blend(uint32_t attachment_count) {
    PipelineColorBlend color_blend;
    color_blend.logicOpEnable_ = VK_FALSE;
    color_blend.attachments_.resize(attachment_count, create_additive_blend_attachment());
    return color_blend;
}

} // namespace VulkanEngine::RAII::PipelineDefaults


