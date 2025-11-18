#include "Timer.hpp"

#include <chrono>
#include <cstdint>
#include <iostream>

namespace VulkanEngine::RAII::Utils {

Timer::Timer() noexcept = default;

void Timer::start() noexcept {
    if (running_) return;
    start_point_ = clock::now();
    running_ = true;
}

void Timer::stop() noexcept {
    if (!running_) return;
    auto end = clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_point_).count();
    accumulated_ns_ += static_cast<std::uint64_t>(interval);
    running_ = false;
}

void Timer::reset() noexcept {
    accumulated_ns_ = 0;
    running_ = false;
}

std::uint64_t Timer::elapsed_nanoseconds() const noexcept {
    if (running_) {
        auto now = clock::now();
        auto interval = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_point_).count();
        return accumulated_ns_ + static_cast<std::uint64_t>(interval);
    }
    return accumulated_ns_;
}

double Timer::elapsed_seconds() const noexcept {
    return static_cast<double>(elapsed_nanoseconds()) / 1'000'000'000.0;
}

ScopedTimer::ScopedTimer(const char* label) noexcept : label_(label), timer_() {
    timer_.start();
}

ScopedTimer::~ScopedTimer() {
    timer_.stop();
    // Minimal logging; could be replaced with more sophisticated debug utils if desired.
    std::cout << "[ScopedTimer] " << (label_ ? label_ : "(unnamed)")
              << " took " << timer_.elapsed_microseconds() << " us\n";
}

} // namespace VulkanEngine::RAII::Utils
