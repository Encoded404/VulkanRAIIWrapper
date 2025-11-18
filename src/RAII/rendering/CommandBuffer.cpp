#include "CommandBuffer.hpp"

#include "CommandPool.hpp"
#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

CommandBuffer::CommandBuffer(VkCommandBuffer command_buffer, const CommandPool& command_pool)
        : commandBuffer_(command_buffer),
            commandPool_(command_pool.get_handle()),
            device_(command_pool.get_device()),
            ownsCommandBuffer_(false) {}

CommandBuffer::CommandBuffer(const CommandPool& command_pool, VkCommandBufferLevel level)
        : commandPool_(command_pool.get_handle()),
            device_(command_pool.get_device()),
      ownsCommandBuffer_(true) {
    VkCommandBufferAllocateInfo alloc_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandPool = commandPool_;
    alloc_info.level = level;
    alloc_info.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device_, &alloc_info, &commandBuffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer");
    }
}

CommandBuffer::~CommandBuffer() {
    cleanup();
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
    : commandBuffer_(other.commandBuffer_),
      commandPool_(other.commandPool_),
      device_(other.device_),
      ownsCommandBuffer_(other.ownsCommandBuffer_) {
    other.commandBuffer_ = VK_NULL_HANDLE;
    other.commandPool_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.ownsCommandBuffer_ = false;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept {
    if (this != &other) {
        cleanup();
        commandBuffer_ = other.commandBuffer_;
        commandPool_ = other.commandPool_;
        device_ = other.device_;
        ownsCommandBuffer_ = other.ownsCommandBuffer_;
        other.commandBuffer_ = VK_NULL_HANDLE;
        other.commandPool_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.ownsCommandBuffer_ = false;
    }
    return *this;
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags,
                          const VkCommandBufferInheritanceInfo* inheritance_info) const {
    VkCommandBufferBeginInfo begin_info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = flags;
    begin_info.pInheritanceInfo = inheritance_info;

    if (vkBeginCommandBuffer(commandBuffer_, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer");
    }
}

void CommandBuffer::end() const {
    if (vkEndCommandBuffer(commandBuffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }
}

void CommandBuffer::reset(VkCommandBufferResetFlags flags) const {
    vkResetCommandBuffer(commandBuffer_, flags);
}

void CommandBuffer::bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) const {
    vkCmdBindPipeline(commandBuffer_, bind_point, pipeline);
}

void CommandBuffer::bind_descriptor_sets(VkPipelineBindPoint bind_point,
                                       VkPipelineLayout layout,
                                       uint32_t first_set,
                                       const std::vector<VkDescriptorSet>& descriptor_sets,
                                       const std::vector<uint32_t>& dynamic_offsets) const {
    vkCmdBindDescriptorSets(commandBuffer_,
                            bind_point,
                            layout,
                            first_set,
                            static_cast<uint32_t>(descriptor_sets.size()),
                            descriptor_sets.data(),
                            static_cast<uint32_t>(dynamic_offsets.size()),
                            dynamic_offsets.empty() ? nullptr : dynamic_offsets.data());
}

void CommandBuffer::bind_vertex_buffers(uint32_t first_binding,
                                      const std::vector<VkBuffer>& buffers,
                                      const std::vector<VkDeviceSize>& offsets) const {
    vkCmdBindVertexBuffers(commandBuffer_,
                           first_binding,
                           static_cast<uint32_t>(buffers.size()),
                           buffers.data(),
                           offsets.data());
}

void CommandBuffer::bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType index_type) const {
    vkCmdBindIndexBuffer(commandBuffer_, buffer, offset, index_type);
}

void CommandBuffer::draw(uint32_t vertex_count,
                         uint32_t instance_count,
                         uint32_t first_vertex,
                         uint32_t first_instance) const {
    vkCmdDraw(commandBuffer_, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer::draw_indexed(uint32_t index_count,
                                uint32_t instance_count,
                                uint32_t first_index,
                                int32_t vertex_offset,
                                uint32_t first_instance) const {
    vkCmdDrawIndexed(commandBuffer_, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandBuffer::begin_render_pass(const VkRenderPassBeginInfo& render_pass_begin,
                                    VkSubpassContents contents) const {
    vkCmdBeginRenderPass(commandBuffer_, &render_pass_begin, contents);
}

void CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(commandBuffer_);
}

void CommandBuffer::next_subpass(VkSubpassContents contents) const {
    vkCmdNextSubpass(commandBuffer_, contents);
}

void CommandBuffer::pipeline_barrier(VkPipelineStageFlags src_stage_mask,
                                    VkPipelineStageFlags dst_stage_mask,
                                    VkDependencyFlags dependency_flags,
                                    const std::vector<VkMemoryBarrier>& memory_barriers,
                                    const std::vector<VkBufferMemoryBarrier>& buffer_memory_barriers,
                                    const std::vector<VkImageMemoryBarrier>& image_memory_barriers) const {
    vkCmdPipelineBarrier(commandBuffer_,
                         src_stage_mask,
                         dst_stage_mask,
                         dependency_flags,
                         static_cast<uint32_t>(memory_barriers.size()),
                         memory_barriers.empty() ? nullptr : memory_barriers.data(),
                         static_cast<uint32_t>(buffer_memory_barriers.size()),
                         buffer_memory_barriers.empty() ? nullptr : buffer_memory_barriers.data(),
                         static_cast<uint32_t>(image_memory_barriers.size()),
                         image_memory_barriers.empty() ? nullptr : image_memory_barriers.data());
}

void CommandBuffer::copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, const std::vector<VkBufferCopy>& regions) const {
    vkCmdCopyBuffer(commandBuffer_,
                    src_buffer,
                    dst_buffer,
                    static_cast<uint32_t>(regions.size()),
                    regions.empty() ? nullptr : regions.data());
}

void CommandBuffer::copy_image(VkImage src_image,
                              VkImageLayout src_image_layout,
                              VkImage dst_image,
                              VkImageLayout dst_image_layout,
                              const std::vector<VkImageCopy>& regions) const {
    vkCmdCopyImage(commandBuffer_,
                   src_image,
                   src_image_layout,
                   dst_image,
                   dst_image_layout,
                   static_cast<uint32_t>(regions.size()),
                   regions.empty() ? nullptr : regions.data());
}

void CommandBuffer::copy_buffer_to_image(VkBuffer src_buffer,
                                      VkImage dst_image,
                                      VkImageLayout dst_image_layout,
                                      const std::vector<VkBufferImageCopy>& regions) const {
    vkCmdCopyBufferToImage(commandBuffer_,
                           src_buffer,
                           dst_image,
                           dst_image_layout,
                           static_cast<uint32_t>(regions.size()),
                           regions.empty() ? nullptr : regions.data());
}

void CommandBuffer::push_constants(VkPipelineLayout layout,
                                  VkShaderStageFlags stage_flags,
                                  uint32_t offset,
                                  uint32_t size,
                                  const void* values) const {
    vkCmdPushConstants(commandBuffer_, layout, stage_flags, offset, size, values);
}

void CommandBuffer::cleanup() {
    // Only free if this wrapper actually owns the command buffer
    if (ownsCommandBuffer_ && commandBuffer_ != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer_);
    }
    commandBuffer_ = VK_NULL_HANDLE;
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

