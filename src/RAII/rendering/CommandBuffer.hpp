#ifndef VULKAN_RAII_RENDERING_COMMAND_BUFFER_HPP
#define VULKAN_RAII_RENDERING_COMMAND_BUFFER_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

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

    [[nodiscard]] VkCommandBuffer get_handle() const { return commandBuffer_; }
    
    // Implicit conversion to VkCommandBuffer
    operator VkCommandBuffer() const { return commandBuffer_; }

    // Check if the command buffer is valid
    [[nodiscard]] bool is_valid() const { return commandBuffer_ != VK_NULL_HANDLE; }

    // Begin recording commands
    void begin(VkCommandBufferUsageFlags flags = 0, 
               const VkCommandBufferInheritanceInfo* inheritance_info = nullptr) const;

    // End recording commands
    void end() const;

    // Reset the command buffer
    void reset(VkCommandBufferResetFlags flags = 0) const;

    // Bind pipeline
    void bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const;

    // Bind descriptor sets
    void bind_descriptor_sets(VkPipelineBindPoint bind_point,
                           VkPipelineLayout layout,
                           uint32_t first_set,
                           const std::vector<VkDescriptorSet>& descriptor_sets,
                           const std::vector<uint32_t>& dynamic_offsets = {}) const;

    // Bind vertex buffers
    void bind_vertex_buffers(uint32_t first_binding,
                          const std::vector<VkBuffer>& buffers,
                          const std::vector<VkDeviceSize>& offsets) const;

    // Bind index buffer
    void bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type) const;

    // Draw commands
    void draw(uint32_t vertex_count, uint32_t instance_count = 1, 
              uint32_t first_vertex = 0, uint32_t first_instance = 0) const;

    void draw_indexed(uint32_t index_count, uint32_t instance_count = 1,
                     uint32_t first_index = 0, int32_t vertex_offset = 0, 
                     uint32_t first_instance = 0) const;

    // Render pass commands
    void begin_render_pass(const VkRenderPassBeginInfo& render_pass_begin, 
                        VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

    void end_render_pass() const;

    void next_subpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;

    // Memory barriers
    void pipeline_barrier(VkPipelineStageFlags src_stage_mask,
                        VkPipelineStageFlags dst_stage_mask,
                        VkDependencyFlags dependency_flags,
                        const std::vector<VkMemoryBarrier>& memory_barriers = {},
                        const std::vector<VkBufferMemoryBarrier>& buffer_memory_barriers = {},
                        const std::vector<VkImageMemoryBarrier>& image_memory_barriers = {}) const;

    // Copy commands
    void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, 
                   const std::vector<VkBufferCopy>& regions) const;

    void copy_image(VkImage src_image, VkImageLayout src_image_layout,
                   VkImage dst_image, VkImageLayout dst_image_layout,
                   const std::vector<VkImageCopy>& regions) const;

    void copy_buffer_to_image(VkBuffer src_buffer, VkImage dst_image, 
                          VkImageLayout dst_image_layout,
                          const std::vector<VkBufferImageCopy>& regions) const;

    // Push constants
    void push_constants(VkPipelineLayout layout, VkShaderStageFlags stage_flags,
                      uint32_t offset, uint32_t size, const void* values) const;

private:
    VkCommandBuffer commandBuffer_{VK_NULL_HANDLE};
    VkCommandPool commandPool_{VK_NULL_HANDLE}; // Reference to command pool for cleanup
    VkDevice device_{VK_NULL_HANDLE}; // Device used for allocation/free
    bool ownsCommandBuffer_; // Whether we allocated this command buffer, dont set default since this is dependent on constructor used

    // Helper methods
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RENDERING_COMMAND_BUFFER_HPP