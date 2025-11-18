#ifndef VULKAN_RAII_RENDERING_PIPELINE_HPP
#define VULKAN_RAII_RENDERING_PIPELINE_HPP

#include <volk.h>
#include <vector>
#include "PipelineStructs.hpp"


namespace VulkanEngine::RAII {

class Device; // Forward declaration
class RenderPass; // Forward declaration

class Pipeline {
public:
    // Constructor for graphics pipeline
    Pipeline(const Device& device,
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
             const std::vector<VkDynamicState>& dynamic_states = {},
             uint32_t subpass = 0,
             VkPipeline base_pipeline = VK_NULL_HANDLE,
             int32_t base_pipeline_index = -1);

    // Constructor for graphics pipeline with tessellation
    Pipeline(const Device& device,
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
             const std::vector<VkDynamicState>& dynamic_states = {},
             uint32_t subpass = 0,
             VkPipeline base_pipeline = VK_NULL_HANDLE,
             int32_t base_pipeline_index = -1);

    // Constructor for compute pipeline
    Pipeline(const Device& device,
             VkPipelineLayout layout,
             const PipelineShaderStage& compute_stage,
             VkPipeline base_pipeline = VK_NULL_HANDLE,
             int32_t base_pipeline_index = -1);

    // Destructor
    ~Pipeline();

    // Move constructor and assignment
    Pipeline(Pipeline&& other) noexcept;
    Pipeline& operator=(Pipeline&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkPipeline by only allowing moving.
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    [[nodiscard]] VkPipeline get_handle() const { return pipeline_; }
    
    // Implicit conversion to VkPipeline
    operator VkPipeline() const { return pipeline_; }

    // Check if the pipeline is valid
    [[nodiscard]] bool is_valid() const { return pipeline_ != VK_NULL_HANDLE; }

    // Get pipeline layout
    [[nodiscard]] VkPipelineLayout get_layout() const { return layout_; }

    // Check pipeline type
    [[nodiscard]] bool is_graphics_pipeline() const { return type_ == Type::GRAPHICS; }
    [[nodiscard]] bool is_compute_pipeline() const { return type_ == Type::COMPUTE; }

    // Get bind point
    [[nodiscard]] VkPipelineBindPoint get_bind_point() const {
        return is_graphics_pipeline() ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE;
    }

private:
    enum class Type {
        GRAPHICS,
        COMPUTE
    };

    VkPipeline pipeline_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    VkPipelineLayout layout_{VK_NULL_HANDLE}; // Reference to layout
    Type type_; // Intentionally no default: all ctors must set this

    // Helper methods
    void create_graphics_pipeline(const RenderPass& render_pass,
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
                               int32_t base_pipeline_index);

    void create_graphics_pipeline_with_tessellation(const RenderPass& render_pass,
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
                                               int32_t base_pipeline_index);

    void create_compute_pipeline(const PipelineShaderStage& compute_stage,
                              VkPipeline base_pipeline,
                              int32_t base_pipeline_index);

    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_PIPELINE_HPP