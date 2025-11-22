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

    [[nodiscard]] VkPipelineLayout GetHandle() const { return pipelineLayout_; }
    
    // Implicit conversion to VkPipelineLayout
    operator VkPipelineLayout() const { return pipelineLayout_; }

    // Check if the pipeline layout is valid
    [[nodiscard]] bool IsValid() const { return pipelineLayout_ != VK_NULL_HANDLE; }

    // Get descriptor set layout count
    [[nodiscard]] uint32_t GetSetLayoutCount() const { return static_cast<uint32_t>(setLayouts_.size()); }

    // Get push constant range count
    [[nodiscard]] uint32_t GetPushConstantRangeCount() const { return static_cast<uint32_t>(pushConstantRanges_.size()); }

    // Get descriptor set layouts
    [[nodiscard]] const std::vector<VkDescriptorSetLayout>& GetSetLayouts() const { return setLayouts_; }

    // Get push constant ranges
    [[nodiscard]] const std::vector<VkPushConstantRange>& GetPushConstantRanges() const { return pushConstantRanges_; }

    // Helper to create simple layout with single descriptor set
    static PipelineLayout CreateSingleSet(const Device& device, VkDescriptorSetLayout set_layout);

    // Helper to create layout with push constants only
    static PipelineLayout CreatePushConstantsOnly(const Device& device,
                                                  const std::vector<VkPushConstantRange>& push_constant_ranges);

    // Helper to create empty layout
    static PipelineLayout CreateEmpty(const Device& device);

private:
    VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE};
    VkDevice device_{VK_NULL_HANDLE}; // Reference to device
    std::vector<VkDescriptorSetLayout> setLayouts_;
    std::vector<VkPushConstantRange> pushConstantRanges_;

    // Helper methods
    void CreatePipelineLayout();
    void Cleanup();
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_RESOURCES_PIPELINE_LAYOUT_HPP