#ifndef VULKAN_RAII_TYPES_QUEUEFAMILYINDICES_HPP
#define VULKAN_RAII_TYPES_QUEUEFAMILYINDICES_HPP

#include <optional>
#include <set>
#include <cstdint>

struct QueueFamilyIndices {
public:
    std::optional<uint32_t> graphicsFamily_; // NOLINT
    std::optional<uint32_t> presentFamily_;  // NOLINT
    std::optional<uint32_t> computeFamily_;  // NOLINT
    std::optional<uint32_t> transferFamily_; // NOLINT
    [[nodiscard]] bool is_complete() const {
        return graphicsFamily_.has_value() && presentFamily_.has_value();
    }

    [[nodiscard]] std::set<uint32_t> get_unique_indices() const {
        std::set<uint32_t> indices;
        if (graphicsFamily_.has_value()) indices.insert(graphicsFamily_.value());
        if (presentFamily_.has_value()) indices.insert(presentFamily_.value());
        if (computeFamily_.has_value()) indices.insert(computeFamily_.value());
        if (transferFamily_.has_value()) indices.insert(transferFamily_.value());
        return indices;
    }
};

#endif // VULKAN_RAII_TYPES_QUEUEFAMILYINDICES_HPP