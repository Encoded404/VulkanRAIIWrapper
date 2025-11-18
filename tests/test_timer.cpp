#include <catch2/catch_test_macros.hpp>
#include "utils/Timer.hpp"
#include <thread>
#include <chrono>

using namespace VulkanEngine::RAII::Utils;

TEST_CASE("Timer basic start/stop") {
    Timer t;
    REQUIRE_FALSE(t.is_running());
    REQUIRE(t.elapsed_nanoseconds() == 0);

    t.start();
    REQUIRE(t.is_running());
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    t.stop();
    REQUIRE_FALSE(t.is_running());
    auto ns = t.elapsed_nanoseconds();
    REQUIRE(ns > 0);
    REQUIRE(t.elapsed_milliseconds() >= 10.0); // allow scheduling slop
}

TEST_CASE("Timer accumulation across cycles") {
    Timer t;
    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    t.stop();
    auto first = t.elapsed_nanoseconds();
    REQUIRE(first > 0);

    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    t.stop();
    auto total = t.elapsed_nanoseconds();
    REQUIRE(total > first);
    REQUIRE(t.elapsed_milliseconds() >= 10.0);
}

TEST_CASE("Timer running elapsed query") {
    Timer t;
    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
    auto mid = t.elapsed_microseconds();
    REQUIRE(mid > 0);
    REQUIRE(t.is_running());
    t.stop();
    auto final_us = t.elapsed_microseconds();
    REQUIRE(final_us >= mid);
}

TEST_CASE("ScopedTimer logs on destruction") {
    // We mainly ensure it doesn't crash; output is not captured here.
    {
        ScopedTimer st("test-scope");
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    REQUIRE(true); // placeholder assertion
}
