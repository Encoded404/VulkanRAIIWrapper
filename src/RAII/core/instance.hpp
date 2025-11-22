#ifndef VULKAN_RAII_CORE_INSTANCE_HPP
#define VULKAN_RAII_CORE_INSTANCE_HPP

#include <vector>
#include <string>
#include <memory>
#include <volk.h>


namespace VulkanEngine::RAII {

class Instance {
public:
    // Constructor that creates a Vulkan instance
    Instance(const std::string& application_name = "VulkanEngine Application",
             uint32_t application_version = VK_MAKE_VERSION(1, 0, 0),
             const std::vector<const char*>& required_extensions = {},
             const std::vector<const char*>& validation_layers = {});

    // Destructor
    ~Instance();

    // Move constructor and assignment
    Instance(Instance&& other) noexcept;
    Instance& operator=(Instance&& other) noexcept;

    // Delete copy constructor and assignment. this ensures unique ownership of the VkInstance by only allowing moving.
    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

    [[nodiscard]] VkInstance GetHandle() const { return instance_; }
    
    // Implicit conversion to VkInstance
    operator VkInstance() const { return instance_; }

    // Check if the instance is valid
    [[nodiscard]] bool IsValid() const { return instance_ != VK_NULL_HANDLE; }

    // Get available extensions
    static std::vector<VkExtensionProperties> GetAvailableExtensions();

    // Get available layers
    static std::vector<VkLayerProperties> GetAvailableLayers();

    // Check if extension is supported
    static bool IsExtensionSupported(const std::string& extension);

    // Check if layer is supported
    static bool IsLayerSupported(const std::string& layer);

private:
    VkInstance instance_{VK_NULL_HANDLE};

    // Helper methods
    void CreateInstance(const std::string& application_name,
                       uint32_t application_version,
                       const std::string& engine_name,
                       uint32_t engine_version,
                       uint32_t api_version,
                       const std::vector<const char*>& required_extensions,
                       const std::vector<const char*>& validation_layers);
    
    bool CheckValidationLayerSupport(const std::vector<const char*>& validation_layers);
};

} // namespace VulkanEngine::RAII


#endif // VULKAN_RAII_CORE_INSTANCE_HPP