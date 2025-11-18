#ifndef VULKAN_RAII_RENDERING_PIPELINE_STRUCTS_HPP
#define VULKAN_RAII_RENDERING_PIPELINE_STRUCTS_HPP

#include <volk.h>
#include <vector>
#include <string>


namespace VulkanEngine::RAII {

struct PipelineShaderStage {
    VkShaderStageFlagBits stage_;
    VkShaderModule module_;
    std::string entryPoint_ = "main";
    const VkSpecializationInfo* specializationInfo_ = nullptr;
};

struct PipelineVertexInput {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions_;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions_;
};

struct PipelineInputAssembly {
    VkPrimitiveTopology topology_ = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 primitiveRestartEnable_ = VK_FALSE;
};

struct PipelineViewport {
    std::vector<VkViewport> viewports_;
    std::vector<VkRect2D> scissors_;
};

struct PipelineRasterization {
    VkBool32 depthClampEnable_ = VK_FALSE;
    VkBool32 rasterizerDiscardEnable_ = VK_FALSE;
    VkPolygonMode polygonMode_ = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode_ = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace_ = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkBool32 depthBiasEnable_ = VK_FALSE;
    float depthBiasConstantFactor_ = 0.0f;
    float depthBiasClamp_ = 0.0f;
    float depthBiasSlopeFactor_ = 0.0f;
    float lineWidth_ = 1.0f;
};

struct PipelineMultisample {
    VkSampleCountFlagBits rasterizationSamples_ = VK_SAMPLE_COUNT_1_BIT;
    VkBool32 sampleShadingEnable_ = VK_FALSE;
    float minSampleShading_ = 1.0f;
    const VkSampleMask* sampleMask_ = nullptr;
    VkBool32 alphaToCoverageEnable_ = VK_FALSE;
    VkBool32 alphaToOneEnable_ = VK_FALSE;
};

struct PipelineDepthStencil {
    VkBool32 depthTestEnable_ = VK_TRUE;
    VkBool32 depthWriteEnable_ = VK_TRUE;
    VkCompareOp depthCompareOp_ = VK_COMPARE_OP_LESS;
    VkBool32 depthBoundsTestEnable_ = VK_FALSE;
    VkBool32 stencilTestEnable_ = VK_FALSE;
    VkStencilOpState front_{};
    VkStencilOpState back_{};
    float minDepthBounds_ = 0.0f;
    float maxDepthBounds_ = 1.0f;
};

struct PipelineColorBlend {
    VkBool32 logicOpEnable_ = VK_FALSE;
    VkLogicOp logicOp_ = VK_LOGIC_OP_COPY;
    std::vector<VkPipelineColorBlendAttachmentState> attachments_;
    float blendConstants_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct PipelineTessellation {
    uint32_t patchControlPoints_ = 3;
};

struct PipelineGeometry {
    // Currently no specific geometry shader state needed
    // This struct exists for future extensibility
};

// Helper functions for creating common pipeline states
namespace PipelineDefaults {

PipelineVertexInput create_empty_vertex_input();
PipelineInputAssembly create_triangle_list_input_assembly();
PipelineInputAssembly create_triangle_strip_input_assembly();
PipelineInputAssembly create_point_list_input_assembly();
PipelineInputAssembly create_line_list_input_assembly();

PipelineRasterization create_default_rasterization();
PipelineRasterization create_wireframe_rasterization();
PipelineRasterization create_no_cull_rasterization();

PipelineMultisample create_no_multisample();
PipelineMultisample create_msa_a4x();
PipelineMultisample create_msa_a8x();

PipelineDepthStencil create_default_depth_stencil();
PipelineDepthStencil create_no_depth_test();
PipelineDepthStencil create_depth_only_test();

PipelineColorBlend create_opaque_color_blend(uint32_t attachment_count = 1);
PipelineColorBlend create_alpha_blend_color_blend(uint32_t attachment_count = 1);
PipelineColorBlend create_additive_blend_color_blend(uint32_t attachment_count = 1);

VkPipelineColorBlendAttachmentState create_opaque_color_blend_attachment();
VkPipelineColorBlendAttachmentState create_alpha_blend_attachment();
VkPipelineColorBlendAttachmentState create_additive_blend_attachment();

} // namespace PipelineDefaults

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_PIPELINE_STRUCTS_HPP