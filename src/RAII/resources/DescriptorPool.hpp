#ifndef VULKAN_RAII_RESOURCES_DESCRIPTOR_POOL_HPP
#define VULKAN_RAII_RESOURCES_DESCRIPTOR_POOL_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration
class DescriptorSetLayout; // Forward declaration

class DescriptorPool {
public:
    // Constructor that creates a descriptor pool
    DescriptorPool(const Device& device,
                   uint32_t max_sets,
                   const std::vector<VkDescriptorPoolSize>& pool_sizes,
                   VkDescriptorPoolCreateFlags flags = 0);

    // Constructor with single descriptor type
    DescriptorPool(const Device& device,
                   uint32_t max_sets,
                   VkDescriptorType descriptor_type,
                   uint32_t descriptor_count,
                   VkDescriptorPoolCreateFlags flags = 0);

    // Destructor
    ~DescriptorPool();

    // Move constructor and assignment
    DescriptorPool(DescriptorPool&& other) noexcept;
    DescriptorPool& operator=(DescriptorPool&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkDescriptorPool by only allowing moving.
    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;

    [[nodiscard]] VkDescriptorPool get_handle() const { return descriptorPool_; }
    
    // Implicit conversion to VkDescriptorPool
    operator VkDescriptorPool() const { return descriptorPool_; }

    // Check if the descriptor pool is valid
    [[nodiscard]] bool is_valid() const { return descriptorPool_ != VK_NULL_HANDLE; }

    // Allocate descriptor sets
    std::vector<VkDescriptorSet> allocate_descriptor_sets(const std::vector<VkDescriptorSetLayout>& layouts);

    // Allocate single descriptor set
    VkDescriptorSet allocate_descriptor_set(VkDescriptorSetLayout layout);

    // Update descriptor sets (simple wrapper around vkUpdateDescriptorSets)
    void update_descriptor_sets(const std::vector<VkWriteDescriptorSet>& writes,
                                const std::vector<VkCopyDescriptorSet>& copies = {}) const;

    // Free descriptor sets (only if pool was created with FREE_DESCRIPTOR_SET_BIT)
    void free_descriptor_sets(const std::vector<VkDescriptorSet>& descriptor_sets);

    // Free single descriptor set
    void free_descriptor_set(VkDescriptorSet descriptor_set);

    // Reset descriptor pool
    void reset(VkDescriptorPoolResetFlags flags = 0);

    // Get max sets
    [[nodiscard]] uint32_t get_max_sets() const { return maxSets_; }

    // Get pool sizes
    [[nodiscard]] const std::vector<VkDescriptorPoolSize>& get_pool_sizes() const { return poolSizes_; }

    // Check if pool allows individual descriptor set freeing
    [[nodiscard]] bool allows_individual_free() const { return allowsIndividualFree_; }

    // Helper to create pool for common use cases
    static DescriptorPool create_for_uniform_buffers(const Device& device,
                                                  uint32_t max_sets,
                                                  uint32_t uniform_buffer_count = 1);

    static DescriptorPool create_for_textures(const Device& device,
                                           uint32_t max_sets,
                                           uint32_t texture_count = 1);

    static DescriptorPool create_for_mixed(const Device& device,
                                        uint32_t max_sets,
                                        uint32_t uniform_buffer_count = 1,
                                        uint32_t sampler_count = 1,
                                        uint32_t storage_buffer_count = 0);

private:
    VkDescriptorPool descriptorPool_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    uint32_t maxSets_{0};
    std::vector<VkDescriptorPoolSize> poolSizes_;
    bool allowsIndividualFree_{false};

    // Helper methods
    void create_descriptor_pool(VkDescriptorPoolCreateFlags flags);
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_DESCRIPTOR_POOL_HPP