#include "PipelineStructs.hpp"
#include <cstdint>



namespace VulkanEngine::RAII::PipelineDefaults {

PipelineVertexInput CreateEmptyVertexInput() {
    return {};
}

PipelineInputAssembly CreateTriangleListInputAssembly() {
    PipelineInputAssembly assembly;
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assembly.primitiveRestartEnable = VK_FALSE;
    return assembly;
}

PipelineInputAssembly CreateTriangleStripInputAssembly() {
    PipelineInputAssembly assembly;
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    assembly.primitiveRestartEnable = VK_TRUE;
    return assembly;
}

PipelineInputAssembly CreatePointListInputAssembly() {
    PipelineInputAssembly assembly;
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    assembly.primitiveRestartEnable = VK_FALSE;
    return assembly;
}

PipelineInputAssembly CreateLineListInputAssembly() {
    PipelineInputAssembly assembly;
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    assembly.primitiveRestartEnable = VK_FALSE;
    return assembly;
}

PipelineRasterization CreateDefaultRasterization() {
    PipelineRasterization rasterization;
    rasterization.depthClampEnable = VK_FALSE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.depthBiasEnable = VK_FALSE;
    rasterization.lineWidth = 1.0f;
    return rasterization;
}

PipelineRasterization CreateWireframeRasterization() {
    PipelineRasterization rasterization = CreateDefaultRasterization();
    rasterization.polygonMode = VK_POLYGON_MODE_LINE;
    rasterization.lineWidth = 1.0f;
    return rasterization;
}

PipelineRasterization CreateNoCullRasterization() {
    PipelineRasterization rasterization = CreateDefaultRasterization();
    rasterization.cullMode = VK_CULL_MODE_NONE;
    return rasterization;
}

PipelineMultisample CreateNoMultisample() {
    PipelineMultisample multisample;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable = VK_FALSE;
    return multisample;
}

PipelineMultisample CreateMsaA4x() {
    PipelineMultisample multisample = CreateNoMultisample();
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    return multisample;
}

PipelineMultisample CreateMsaA8x() {
    PipelineMultisample multisample = CreateNoMultisample();
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
    return multisample;
}

PipelineDepthStencil CreateDefaultDepthStencil() {
    PipelineDepthStencil depth_stencil;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.0f;
    depth_stencil.maxDepthBounds = 1.0f;
    return depth_stencil;
}

PipelineDepthStencil CreateNoDepthTest() {
    PipelineDepthStencil depth_stencil = CreateDefaultDepthStencil();
    depth_stencil.depthTestEnable = VK_FALSE;
    depth_stencil.depthWriteEnable = VK_FALSE;
    return depth_stencil;
}

PipelineDepthStencil CreateDepthOnlyTest() {
    PipelineDepthStencil depth_stencil = CreateDefaultDepthStencil();
    depth_stencil.stencilTestEnable = VK_FALSE;
    return depth_stencil;
}

VkPipelineColorBlendAttachmentState CreateOpaqueColorBlendAttachment() {
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

VkPipelineColorBlendAttachmentState CreateAlphaBlendAttachment() {
    VkPipelineColorBlendAttachmentState attachment = CreateOpaqueColorBlendAttachment();
    attachment.blendEnable = VK_TRUE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return attachment;
}

VkPipelineColorBlendAttachmentState CreateAdditiveBlendAttachment() {
    VkPipelineColorBlendAttachmentState attachment = CreateOpaqueColorBlendAttachment();
    attachment.blendEnable = VK_TRUE;
    attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.colorBlendOp = VK_BLEND_OP_ADD;
    attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return attachment;
}

PipelineColorBlend CreateOpaqueColorBlend(uint32_t attachment_count) {
    PipelineColorBlend color_blend;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.attachments.resize(attachment_count, CreateOpaqueColorBlendAttachment());
    return color_blend;
}

PipelineColorBlend CreateAlphaBlendColorBlend(uint32_t attachment_count) {
    PipelineColorBlend color_blend;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.attachments.resize(attachment_count, CreateAlphaBlendAttachment());
    return color_blend;
}

PipelineColorBlend CreateAdditiveBlendColorBlend(uint32_t attachment_count) {
    PipelineColorBlend color_blend;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.attachments.resize(attachment_count, CreateAdditiveBlendAttachment());
    return color_blend;
}

} // namespace VulkanEngine::RAII::PipelineDefaults


