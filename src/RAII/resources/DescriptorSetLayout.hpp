#ifndef VULKAN_RAII_RESOURCES_DESCRIPTOR_SET_LAYOUT_HPP
#define VULKAN_RAII_RESOURCES_DESCRIPTOR_SET_LAYOUT_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

struct DescriptorSetLayoutBinding {
    uint32_t binding_;
    VkDescriptorType descriptorType_;
    uint32_t descriptorCount_ = 1;
    VkShaderStageFlags stageFlags_;
    const VkSampler* immutableSamplers_ = nullptr;
};

class DescriptorSetLayout {
public:
    // Constructor that creates a descriptor set layout
    DescriptorSetLayout(const Device& device,
                       const std::vector<DescriptorSetLayoutBinding>& bindings,
                       VkDescriptorSetLayoutCreateFlags flags = 0);

    // Constructor with raw Vulkan bindings
    DescriptorSetLayout(const Device& device,
                       const std::vector<VkDescriptorSetLayoutBinding>& bindings,
                       VkDescriptorSetLayoutCreateFlags flags = 0);

    // Destructor
    ~DescriptorSetLayout();

    // Move constructor and assignment
    DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkDescriptorSetLayout by only allowing moving.
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

    [[nodiscard]] VkDescriptorSetLayout get_handle() const { return descriptorSetLayout_; }
    
    // Implicit conversion to VkDescriptorSetLayout
    operator VkDescriptorSetLayout() const { return descriptorSetLayout_; }

    // Check if the descriptor set layout is valid
    [[nodiscard]] bool is_valid() const { return descriptorSetLayout_ != VK_NULL_HANDLE; }

    // Get binding count
    [[nodiscard]] uint32_t get_binding_count() const { return static_cast<uint32_t>(bindings_.size()); }

    // Get bindings
    [[nodiscard]] const std::vector<VkDescriptorSetLayoutBinding>& get_bindings() const { return bindings_; }

    // Check if binding exists
    [[nodiscard]] bool has_binding(uint32_t binding) const;

    // Get binding by index
    [[nodiscard]] const VkDescriptorSetLayoutBinding* get_binding(uint32_t binding) const;

    // Get descriptor type for binding
    [[nodiscard]] VkDescriptorType get_descriptor_type(uint32_t binding) const;

    // Get descriptor count for binding
    [[nodiscard]] uint32_t get_descriptor_count(uint32_t binding) const;

    // Helper to create common layouts
    static DescriptorSetLayout create_single_binding(const Device& device,
                                                   uint32_t binding,
                                                   VkDescriptorType descriptor_type,
                                                   VkShaderStageFlags stage_flags,
                                                   uint32_t descriptor_count = 1);

private:
    VkDescriptorSetLayout descriptorSetLayout_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    std::vector<VkDescriptorSetLayoutBinding> bindings_;

    // Helper methods
    void create_descriptor_set_layout(VkDescriptorSetLayoutCreateFlags flags);
    void cleanup();
    
    // Convert custom bindings to Vulkan bindings
    std::vector<VkDescriptorSetLayoutBinding> convert_bindings(const std::vector<DescriptorSetLayoutBinding>& bindings);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_DESCRIPTOR_SET_LAYOUT_HPP