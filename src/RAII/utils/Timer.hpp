#ifndef VULKAN_RAII_UTILS_TIMER_HPP
#define VULKAN_RAII_UTILS_TIMER_HPP

#include <chrono>
#include <cstdint>
#include <string>

namespace VulkanEngine::RAII::Utils {

// High-resolution timer utility for performance measurements.
// Supports start/stop cycles with accumulation and querying while running.
class Timer {
public:
    Timer() noexcept;

    // Start or resume timing. If already running, does nothing.
    void start() noexcept;
    // Stop timing and accumulate the elapsed interval. If not running, does nothing.
    void stop() noexcept;
    // Reset accumulated time and running state.
    void reset() noexcept;

    [[nodiscard]] bool is_running() const noexcept { return running_; }

    // Elapsed time including current running segment (if any).
    [[nodiscard]] double elapsed_seconds() const noexcept;
    [[nodiscard]] double elapsed_milliseconds() const noexcept { return elapsed_seconds() * 1'000.0; }
    [[nodiscard]] double elapsed_microseconds() const noexcept { return elapsed_seconds() * 1'000'000.0; }
    [[nodiscard]] std::uint64_t elapsed_nanoseconds() const noexcept;

private:
    using clock = std::chrono::steady_clock;
    clock::time_point start_point_{};
    std::uint64_t accumulated_ns_{0}; // accumulated nanoseconds across stop cycles
    bool running_{false};
};

// RAII helper that starts a timer on construction and logs the elapsed time
// when destroyed. Intended for scoped performance measurement.
class ScopedTimer {
public:
    explicit ScopedTimer(const char* label) noexcept;
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

private:
    const char* label_{nullptr};
    Timer timer_{};
};

} // namespace VulkanEngine::RAII::Utils

#endif // VULKAN_RAII_UTILS_TIMER_HPP