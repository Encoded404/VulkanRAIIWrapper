#ifndef VULKAN_RAII_RESOURCES_PIPELINE_LAYOUT_HPP
#define VULKAN_RAII_RESOURCES_PIPELINE_LAYOUT_HPP

#include <volk.h>
#include <vector>


namespace VulkanEngine::RAII {

class Device; // Forward declaration

class PipelineLayout {
public:
    // Constructor that creates a pipeline layout
    PipelineLayout(const Device& device,
                   const std::vector<VkDescriptorSetLayout>& set_layouts = {},
                   const std::vector<VkPushConstantRange>& push_constant_ranges = {});

    // Destructor
    ~PipelineLayout();

    // Move constructor and assignment
    PipelineLayout(PipelineLayout&& other) noexcept;
    PipelineLayout& operator=(PipelineLayout&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkPipelineLayout by only allowing moving.
    PipelineLayout(const PipelineLayout&) = delete;
    PipelineLayout& operator=(const PipelineLayout&) = delete;

    [[nodiscard]] VkPipelineLayout get_handle() const { return pipelineLayout_; }
    
    // Implicit conversion to VkPipelineLayout
    operator VkPipelineLayout() const { return pipelineLayout_; }

    // Check if the pipeline layout is valid
    [[nodiscard]] bool is_valid() const { return pipelineLayout_ != VK_NULL_HANDLE; }

    // Get descriptor set layout count
    [[nodiscard]] uint32_t get_set_layout_count() const { return static_cast<uint32_t>(setLayouts_.size()); }

    // Get push constant range count
    [[nodiscard]] uint32_t get_push_constant_range_count() const { return static_cast<uint32_t>(pushConstantRanges_.size()); }

    // Get descriptor set layouts
    [[nodiscard]] const std::vector<VkDescriptorSetLayout>& get_set_layouts() const { return setLayouts_; }

    // Get push constant ranges
    [[nodiscard]] const std::vector<VkPushConstantRange>& get_push_constant_ranges() const { return pushConstantRanges_; }

    // Helper to create simple layout with single descriptor set
    static PipelineLayout create_single_set(const Device& device, VkDescriptorSetLayout set_layout);

    // Helper to create layout with push constants only
    static PipelineLayout create_push_constants_only(const Device& device,
                                                  const std::vector<VkPushConstantRange>& push_constant_ranges);

    // Helper to create empty layout
    static PipelineLayout create_empty(const Device& device);

private:
    VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    std::vector<VkDescriptorSetLayout> setLayouts_;
    std::vector<VkPushConstantRange> pushConstantRanges_;

    // Helper methods
    void create_pipeline_layout();
    void cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_PIPELINE_LAYOUT_HPP