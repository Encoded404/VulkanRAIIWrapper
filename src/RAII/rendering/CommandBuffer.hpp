#ifndef VULKAN_RAII_RENDERING_COMMAND_BUFFER_HPP
#define VULKAN_RAII_RENDERING_COMMAND_BUFFER_HPP

#include <volk.h>
#include <span>


namespace VulkanEngine::RAII {

// forward declare classes Renderer, Swapchain, RenderPass
class Renderer;
class Swapchain;
class RenderPass;
    
class CommandPool; // Forward declaration
class Device; // Forward declaration

class CommandBuffer {
public:
    // Constructor that wraps an allocated VkCommandBuffer
    CommandBuffer(VkCommandBuffer command_buffer, const CommandPool& command_pool);

    // Constructor that allocates a new command buffer from the pool
    CommandBuffer(const CommandPool& command_pool, 
                  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Destructor
    ~CommandBuffer();

    // Move constructor and assignment
    CommandBuffer(CommandBuffer&& other) noexcept;
    CommandBuffer& operator=(CommandBuffer&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkCommandBuffer by only allowing moving.
    CommandBuffer(const CommandBuffer&) = delete;
    CommandBuffer& operator=(const CommandBuffer&) = delete;

    [[nodiscard]] VkCommandBuffer GetHandle() const { return commandBuffer_; }
    
    // Implicit conversion to VkCommandBuffer
    operator VkCommandBuffer() const { return commandBuffer_; }

    // Check if the command buffer is valid
    [[nodiscard]] bool IsValid() const { return commandBuffer_ != VK_NULL_HANDLE; }

    // Begin recording commands
    void Begin(VkCommandBufferUsageFlags flags = 0, 
               const VkCommandBufferInheritanceInfo* inheritance_info = nullptr) const;

    // End recording commands
    void End() const;

    // Reset the command buffer
    void Reset(VkCommandBufferResetFlags flags = 0) const;

    // Bind pipeline
    void BindPipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const;

    // Bind descriptor sets
    void BindDescriptorSets(VkPipelineBindPoint bind_point,
                           VkPipelineLayout layout,
                           uint32_t first_set,
                           std::span<const VkDescriptorSet> descriptor_sets,
                           std::span<const uint32_t> dynamic_offsets = {}) const;

    // Bind vertex buffers
    void BindVertexBuffers(uint32_t first_binding,
                          std::span<const VkBuffer> buffers,
                          std::span<const VkDeviceSize> offsets) const;

    // Bind index buffer
    void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type) const;

    // Draw commands
    void Draw(uint32_t vertex_count, uint32_t instance_count = 1, 
              uint32_t first_vertex = 0, uint32_t first_instance = 0) const;

    void DrawIndexed(uint32_t index_count, uint32_t instance_count = 1,
                     uint32_t first_index = 0, int32_t vertex_offset = 0, 
                     uint32_t first_instance = 0) const;

    // Issue one or many indexed draws based on commands stored in a GPU buffer.
    void DrawIndexedIndirect(VkBuffer buffer,
                               VkDeviceSize offset,
                               uint32_t draw_count,
                               uint32_t stride) const;

    // Render pass commands
    // Standard: single clear value (most common). Builds VkRenderPassBeginInfo internally and calls vkCmdBeginRenderPass
    void BeginRenderPass(VkRenderPass render_pass,
                           const VkExtent2D extent,
                           VkFramebuffer framebuffer,
                           const VkClearValue& clear_value,
                           VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

    // Overload: multiple clear values via span (when available)
    void BeginRenderPass(VkRenderPass render_pass,
                           const VkExtent2D extend,
                           VkFramebuffer framebuffer,
                           std::span<const VkClearValue> clear_values,
                           VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

    // Advanced: directly pass a fully constructed VkRenderPassBeginInfo
    void BeginRenderPass(const VkRenderPassBeginInfo& render_pass_begin, 
                        VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

    void EndRenderPass() const;

    void NextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

    // Memory barriers
    void PipelineBarrier(VkPipelineStageFlags src_stage_mask,
                        VkPipelineStageFlags dst_stage_mask,
                        VkDependencyFlags dependency_flags,
                        std::span<const VkMemoryBarrier> memory_barriers = {},
                        std::span<const VkBufferMemoryBarrier> buffer_memory_barriers = {},
                        std::span<const VkImageMemoryBarrier> image_memory_barriers = {}) const;

    // Copy commands
    void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, 
                   std::span<const VkBufferCopy> regions) const;

    void CopyImage(VkImage src_image, VkImageLayout src_image_layout,
                   VkImage dst_image, VkImageLayout dst_image_layout,
                   std::span<const VkImageCopy> regions) const;

    void CopyBufferToImage(VkBuffer src_buffer, VkImage dst_image, 
                          VkImageLayout dst_image_layout,
                          std::span<const VkBufferImageCopy> regions) const;

    // Push constants
    void PushConstants(VkPipelineLayout layout, VkShaderStageFlags stage_flags,
                      uint32_t offset, uint32_t size, const void* values) const;

    void SetViewport(const VkViewport& viewport) {vkCmdSetViewport(commandBuffer_, 0, 1, &viewport);} // single
    void SetViewports(uint32_t first_viewport, std::span<const VkViewport> viewports) {
        vkCmdSetViewport(commandBuffer_, first_viewport, static_cast<uint32_t>(viewports.size()), viewports.empty() ? nullptr : viewports.data());
    }   // multiple

    void SetScissor(const VkRect2D& scissor) {vkCmdSetScissor(commandBuffer_, 0, 1, &scissor);} // single
    void SetScissors(uint32_t first_scissor, std::span<const VkRect2D> scissors) {
        vkCmdSetScissor(commandBuffer_, first_scissor, static_cast<uint32_t>(scissors.size()), scissors.empty() ? nullptr : scissors.data());
    }   // multiple

private:
    VkCommandBuffer commandBuffer_{VK_NULL_HANDLE};
    VkCommandPool commandPool_{VK_NULL_HANDLE}; // Reference to command pool for cleanup
    VkDevice device_{VK_NULL_HANDLE}; // Device used for allocation/free
    bool ownsCommandBuffer_; // Whether we allocated this command buffer, dont set default since this is dependent on constructor used

    // Helper methods
    void Cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_COMMAND_BUFFER_HPP