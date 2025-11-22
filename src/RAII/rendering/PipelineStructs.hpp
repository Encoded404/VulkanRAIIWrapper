#ifndef VULKAN_RAII_RENDERING_PIPELINE_STRUCTS_HPP
#define VULKAN_RAII_RENDERING_PIPELINE_STRUCTS_HPP

#include <volk.h>
#include <vector>
#include <string>


namespace VulkanEngine::RAII {

struct PipelineShaderStage {
    VkShaderStageFlagBits stage;
    VkShaderModule module;
    std::string entryPoint = "main";
    const VkSpecializationInfo* specializationInfo = nullptr;
};

struct PipelineVertexInput {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

struct PipelineInputAssembly {
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 primitiveRestartEnable = VK_FALSE;
};

struct PipelineViewport {
    std::vector<VkViewport> viewports;
    std::vector<VkRect2D> scissors;
};

struct PipelineRasterization {
    VkBool32 depthClampEnable = VK_FALSE;
    VkBool32 rasterizerDiscardEnable = VK_FALSE;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
    VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    VkBool32 depthBiasEnable = VK_FALSE;
    float depthBiasConstantFactor = 0.0f;
    float depthBiasClamp = 0.0f;
    float depthBiasSlopeFactor = 0.0f;
    float lineWidth = 1.0f;
};

struct PipelineMultisample {
    VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    VkBool32 sampleShadingEnable = VK_FALSE;
    float minSampleShading = 1.0f;
    const VkSampleMask* sampleMask = nullptr;
    VkBool32 alphaToCoverageEnable = VK_FALSE;
    VkBool32 alphaToOneEnable = VK_FALSE;
};

struct PipelineDepthStencil {
    VkBool32 depthTestEnable = VK_TRUE;
    VkBool32 depthWriteEnable = VK_TRUE;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    VkBool32 depthBoundsTestEnable = VK_FALSE;
    VkBool32 stencilTestEnable = VK_FALSE;
    VkStencilOpState front{};
    VkStencilOpState back{};
    float minDepthBounds = 0.0f;
    float maxDepthBounds = 1.0f;
};

struct PipelineColorBlend {
    VkBool32 logicOpEnable = VK_FALSE;
    VkLogicOp logicOp = VK_LOGIC_OP_COPY;
    std::vector<VkPipelineColorBlendAttachmentState> attachments;
    float blendConstants[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

struct PipelineTessellation {
    uint32_t patchControlPoints = 3;
};

struct PipelineGeometry {
    // Currently no specific geometry shader state needed
    // This struct exists for future extensibility
};

// Helper functions for creating common pipeline states
namespace PipelineDefaults {

PipelineVertexInput CreateEmptyVertexInput();
PipelineInputAssembly CreateTriangleListInputAssembly();
PipelineInputAssembly CreateTriangleStripInputAssembly();
PipelineInputAssembly CreatePointListInputAssembly();
PipelineInputAssembly CreateLineListInputAssembly();

PipelineRasterization CreateDefaultRasterization();
PipelineRasterization CreateWireframeRasterization();
PipelineRasterization CreateNoCullRasterization();

PipelineMultisample CreateNoMultisample();
PipelineMultisample CreateMsaA4x();
PipelineMultisample CreateMsaA8x();

PipelineDepthStencil CreateDefaultDepthStencil();
PipelineDepthStencil CreateNoDepthTest();
PipelineDepthStencil CreateDepthOnlyTest();

PipelineColorBlend CreateOpaqueColorBlend(uint32_t attachment_count = 1);
PipelineColorBlend CreateAlphaBlendColorBlend(uint32_t attachment_count = 1);
PipelineColorBlend CreateAdditiveBlendColorBlend(uint32_t attachment_count = 1);

VkPipelineColorBlendAttachmentState CreateOpaqueColorBlendAttachment();
VkPipelineColorBlendAttachmentState CreateAlphaBlendAttachment();
VkPipelineColorBlendAttachmentState CreateAdditiveBlendAttachment();

} // namespace PipelineDefaults

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_PIPELINE_STRUCTS_HPP