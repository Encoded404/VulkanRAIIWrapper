#include "Semaphore.hpp"

#include "../core/Device.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>


namespace VulkanEngine::RAII {

Semaphore::Semaphore(const Device& device, VkSemaphoreCreateFlags flags)
    : device_(device.get_handle()) {
    create_semaphore(flags, nullptr);
}

Semaphore::Semaphore(const Device& device, uint64_t initial_value, VkSemaphoreCreateFlags flags)
    : device_(device.get_handle()), isTimelineSemaphore_(true) {
    create_semaphore(flags, &initial_value);
}

Semaphore::~Semaphore() {
    cleanup();
}

Semaphore::Semaphore(Semaphore&& other) noexcept
    : semaphore_(other.semaphore_),
      device_(other.device_),
      isTimelineSemaphore_(other.isTimelineSemaphore_) {
    other.semaphore_ = VK_NULL_HANDLE;
    other.device_ = VK_NULL_HANDLE;
    other.isTimelineSemaphore_ = false;
}

Semaphore& Semaphore::operator=(Semaphore&& other) noexcept {
    if (this != &other) {
        cleanup();
        semaphore_ = other.semaphore_;
        device_ = other.device_;
        isTimelineSemaphore_ = other.isTimelineSemaphore_;
        other.semaphore_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        other.isTimelineSemaphore_ = false;
    }
    return *this;
}

uint64_t Semaphore::get_counter_value() const {
    if (!isTimelineSemaphore_) {
        throw std::runtime_error("Semaphore is not a timeline semaphore");
    }
    uint64_t value = 0;
    vkGetSemaphoreCounterValue(device_, semaphore_, &value);
    return value;
}

VkResult Semaphore::wait(uint64_t value, uint64_t timeout) const {
    VkSemaphoreWaitInfo wait_info{VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
    wait_info.flags = 0;
    wait_info.semaphoreCount = 1;
    wait_info.pSemaphores = &semaphore_;
    wait_info.pValues = &value;
    return vkWaitSemaphores(device_, &wait_info, timeout);
}

VkResult Semaphore::signal(uint64_t value) const {
    VkSemaphoreSignalInfo signal_info{VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO};
    signal_info.semaphore = semaphore_;
    signal_info.value = value;
    return vkSignalSemaphore(device_, &signal_info);
}

VkResult Semaphore::wait_semaphores(const Device& device,
                                   const std::vector<VkSemaphore>& semaphores,
                                   const std::vector<uint64_t>& values,
                                   bool wait_all,
                                   uint64_t timeout) {
    if (semaphores.size() != values.size()) {
        throw std::runtime_error("Semaphore wait arrays must be the same size");
    }

    VkSemaphoreWaitInfo wait_info{VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO};
    wait_info.flags = wait_all ? 0 : VK_SEMAPHORE_WAIT_ANY_BIT;
    wait_info.semaphoreCount = static_cast<uint32_t>(semaphores.size());
    wait_info.pSemaphores = semaphores.data();
    wait_info.pValues = values.data();

    return vkWaitSemaphores(device.get_handle(), &wait_info, timeout);
}

VkResult Semaphore::signal_semaphores(const Device& device,
                                     const std::vector<VkSemaphore>& semaphores,
                                     const std::vector<uint64_t>& values) {
    if (semaphores.size() != values.size()) {
        throw std::runtime_error("Semaphore signal arrays must be the same size");
    }

    VkSemaphoreSignalInfo signal_info{VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO};
    signal_info.semaphore = VK_NULL_HANDLE;
    signal_info.value = 0;

    VkResult result = VK_SUCCESS;
    for (size_t i = 0; i < semaphores.size(); ++i) {
        signal_info.semaphore = semaphores[i];
        signal_info.value = values[i];
        result = vkSignalSemaphore(device.get_handle(), &signal_info);
        if (result != VK_SUCCESS) {
            break;
        }
    }
    return result;
}

void Semaphore::create_semaphore(VkSemaphoreCreateFlags flags, uint64_t* initial_value) {
    VkSemaphoreCreateInfo semaphore_info{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    semaphore_info.flags = flags;

    VkSemaphoreTypeCreateInfo timeline_info{VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
    if (initial_value) {
        timeline_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timeline_info.initialValue = *initial_value;
        semaphore_info.pNext = &timeline_info;
    }

    if (vkCreateSemaphore(device_, &semaphore_info, nullptr, &semaphore_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create semaphore");
    }
}

void Semaphore::cleanup() {
    if (semaphore_ != VK_NULL_HANDLE) {
        vkDestroySemaphore(device_, semaphore_, nullptr);
        semaphore_ = VK_NULL_HANDLE;
    }
    device_ = VK_NULL_HANDLE;
}

} // namespace VulkanEngine::RAII

