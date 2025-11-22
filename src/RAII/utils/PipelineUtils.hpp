#ifndef VULKAN_RAII_UTILS_PIPELINE_UTILS_HPP
#define VULKAN_RAII_UTILS_PIPELINE_UTILS_HPP

#include <volk.h>

#include <cstdint>
#include <vector>



namespace VulkanEngine::RAII::Utils {

// Pipeline utilities
class PipelineUtils {
public:
    // Create default viewport
    static VkViewport CreateViewport(float width, float height,
                                    float min_depth = 0.0f, float max_depth = 1.0f,
                                    float x = 0.0f, float y = 0.0f);
    
    // Create default scissor rectangle
    static VkRect2D CreateScissor(uint32_t width, uint32_t height,
                                 int32_t offset_x = 0, int32_t offset_y = 0);
    
    // Create default color blend attachment
    static VkPipelineColorBlendAttachmentState CreateDefaultColorBlendAttachment();
    
    // Create alpha blend attachment
    static VkPipelineColorBlendAttachmentState CreateAlphaBlendAttachment();

    // Create additive blend attachment
    static VkPipelineColorBlendAttachmentState CreateAdditiveBlendAttachment();

    // Create no blend attachment (color write disabled)
    static VkPipelineColorBlendAttachmentState CreateNoBlendAttachment();
    
    // Create default depth stencil state
    static VkPipelineDepthStencilStateCreateInfo CreateDefaultDepthStencilState();

    // Create depth test only state (no write)
    static VkPipelineDepthStencilStateCreateInfo CreateDepthTestOnlyState();

    // Create no depth test state
    static VkPipelineDepthStencilStateCreateInfo CreateNoDepthTestState();
    
    // Create default rasterization state
    static VkPipelineRasterizationStateCreateInfo CreateDefaultRasterizationState();

    // Create wireframe rasterization state
    static VkPipelineRasterizationStateCreateInfo CreateWireframeRasterizationState();

    // Create no cull rasterization state
    static VkPipelineRasterizationStateCreateInfo CreateNoCullRasterizationState();
    
    // Create default multisample state
    static VkPipelineMultisampleStateCreateInfo CreateDefaultMultisampleState();

    // Create MSAA multisample state
    static VkPipelineMultisampleStateCreateInfo CreateMsaaMultisampleState(VkSampleCountFlagBits samples);

    // Create vertex input binding description
    static VkVertexInputBindingDescription CreateVertexInputBinding(uint32_t binding,
                                                                   uint32_t stride,
                                                                   VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX);

    // Create vertex input attribute description
    static VkVertexInputAttributeDescription CreateVertexInputAttribute(uint32_t location,
                                                                       uint32_t binding,
                                                                       VkFormat format,
                                                                       uint32_t offset);

    // Create shader stage create info
    static VkPipelineShaderStageCreateInfo CreateShaderStageCreateInfo(VkShaderStageFlagBits stage,
                                                                       VkShaderModule module,
                                                                       const char* entry_point = "main",
                                                                       const VkSpecializationInfo* specialization_info = nullptr);

    // Create input assembly state
    static VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyState(VkPrimitiveTopology topology,
                                                                           VkBool32 primitive_restart_enable = VK_FALSE);

    // Create tessellation state
    static VkPipelineTessellationStateCreateInfo CreateTessellationState(uint32_t patch_control_points);

    // Create viewport state
    static VkPipelineViewportStateCreateInfo CreateViewportState(const std::vector<VkViewport>& viewports,
                                                                 const std::vector<VkRect2D>& scissors);

    // Create dynamic viewport state
    static VkPipelineViewportStateCreateInfo CreateDynamicViewportState(uint32_t viewport_count = 1,
                                                                        uint32_t scissor_count = 1);

    // Create color blend state
    static VkPipelineColorBlendStateCreateInfo CreateColorBlendState(const std::vector<VkPipelineColorBlendAttachmentState>& attachments,
                                                                     VkBool32 logic_op_enable = VK_FALSE,
                                                                     VkLogicOp logic_op = VK_LOGIC_OP_COPY);

    // Create dynamic state
    static VkPipelineDynamicStateCreateInfo CreateDynamicState(const std::vector<VkDynamicState>& dynamic_states);

    // Common dynamic states
    static std::vector<VkDynamicState> GetBasicDynamicStates();
    static std::vector<VkDynamicState> GetExtendedDynamicStates();
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_PIPELINE_UTILS_HPP