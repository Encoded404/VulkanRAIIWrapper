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
    static VkViewport create_viewport(float width, float height,
                                    float min_depth = 0.0f, float max_depth = 1.0f,
                                    float x = 0.0f, float y = 0.0f);
    
    // Create default scissor rectangle
    static VkRect2D create_scissor(uint32_t width, uint32_t height,
                                 int32_t offset_x = 0, int32_t offset_y = 0);
    
    // Create default color blend attachment
    static VkPipelineColorBlendAttachmentState create_default_color_blend_attachment();
    
    // Create alpha blend attachment
    static VkPipelineColorBlendAttachmentState create_alpha_blend_attachment();

    // Create additive blend attachment
    static VkPipelineColorBlendAttachmentState create_additive_blend_attachment();

    // Create no blend attachment (color write disabled)
    static VkPipelineColorBlendAttachmentState create_no_blend_attachment();
    
    // Create default depth stencil state
    static VkPipelineDepthStencilStateCreateInfo create_default_depth_stencil_state();

    // Create depth test only state (no write)
    static VkPipelineDepthStencilStateCreateInfo create_depth_test_only_state();

    // Create no depth test state
    static VkPipelineDepthStencilStateCreateInfo create_no_depth_test_state();
    
    // Create default rasterization state
    static VkPipelineRasterizationStateCreateInfo create_default_rasterization_state();

    // Create wireframe rasterization state
    static VkPipelineRasterizationStateCreateInfo create_wireframe_rasterization_state();

    // Create no cull rasterization state
    static VkPipelineRasterizationStateCreateInfo create_no_cull_rasterization_state();
    
    // Create default multisample state
    static VkPipelineMultisampleStateCreateInfo create_default_multisample_state();

    // Create MSAA multisample state
    static VkPipelineMultisampleStateCreateInfo create_msaa_multisample_state(VkSampleCountFlagBits samples);

    // Create vertex input binding description
    static VkVertexInputBindingDescription create_vertex_input_binding(uint32_t binding,
                                                                   uint32_t stride,
                                                                   VkVertexInputRate input_rate = VK_VERTEX_INPUT_RATE_VERTEX);

    // Create vertex input attribute description
    static VkVertexInputAttributeDescription create_vertex_input_attribute(uint32_t location,
                                                                       uint32_t binding,
                                                                       VkFormat format,
                                                                       uint32_t offset);

    // Create shader stage create info
    static VkPipelineShaderStageCreateInfo create_shader_stage_create_info(VkShaderStageFlagBits stage,
                                                                       VkShaderModule module,
                                                                       const char* entry_point = "main",
                                                                       const VkSpecializationInfo* specialization_info = nullptr);

    // Create input assembly state
    static VkPipelineInputAssemblyStateCreateInfo create_input_assembly_state(VkPrimitiveTopology topology,
                                                                           VkBool32 primitive_restart_enable = VK_FALSE);

    // Create tessellation state
    static VkPipelineTessellationStateCreateInfo create_tessellation_state(uint32_t patch_control_points);

    // Create viewport state
    static VkPipelineViewportStateCreateInfo create_viewport_state(const std::vector<VkViewport>& viewports,
                                                                 const std::vector<VkRect2D>& scissors);

    // Create dynamic viewport state
    static VkPipelineViewportStateCreateInfo create_dynamic_viewport_state(uint32_t viewport_count = 1,
                                                                        uint32_t scissor_count = 1);

    // Create color blend state
    static VkPipelineColorBlendStateCreateInfo create_color_blend_state(const std::vector<VkPipelineColorBlendAttachmentState>& attachments,
                                                                     VkBool32 logic_op_enable = VK_FALSE,
                                                                     VkLogicOp logic_op = VK_LOGIC_OP_COPY);

    // Create dynamic state
    static VkPipelineDynamicStateCreateInfo create_dynamic_state(const std::vector<VkDynamicState>& dynamic_states);

    // Common dynamic states
    static std::vector<VkDynamicState> get_basic_dynamic_states();
    static std::vector<VkDynamicState> get_extended_dynamic_states();
};

} // namespace VulkanEngine::RAII::Utils



#endif // VULKAN_RAII_UTILS_PIPELINE_UTILS_HPP