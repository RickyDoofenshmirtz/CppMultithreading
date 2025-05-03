#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <numbers>
#include <random>
#include <span>
#include <vector>

[[maybe_unused]]
static constexpr bool CHUNK_MEASUREMENT_ENABLED = false;

static constexpr std::size_t WORKER_COUNT = 4;
static constexpr std::size_t CHUNK_SIZE   = 8000;
static constexpr std::size_t CHUNK_COUNT  = 100;

static constexpr std::size_t LIGHT_ITERATIONS = 100;
static constexpr std::size_t HEAVY_ITERATIONS = 1000;
static constexpr double PROBABILITY_HEAVY     = 0.15;

[[maybe_unused]]
static constexpr std::size_t SUBSET_SIZE = CHUNK_SIZE / WORKER_COUNT;

static_assert(CHUNK_SIZE >= WORKER_COUNT);
static_assert(CHUNK_SIZE % WORKER_COUNT == 0);

struct task
{
    double value;
    bool heavy;

    auto process() const -> std::uint32_t
    {
        const auto iterations = (heavy) ? HEAVY_ITERATIONS : LIGHT_ITERATIONS;
        auto intermediate     = value;
        for (std::size_t i{}; i < iterations; ++i)
        {
            const auto digits =
                static_cast<std::uint32_t>(std::abs(
                    std::sin(std::numbers::pi * std::cos(intermediate)) *
                    10'000'000.0)) %
                100'000;
            intermediate = static_cast<double>(digits) / 10'000.0;
        }
        return static_cast<std::uint32_t>(std::exp(intermediate));
    }
};

inline auto generate_data_set() -> std::vector<std::array<task, CHUNK_SIZE>>
{
    std::minstd_rand rne;
    std::uniform_real_distribution v_dist{ 0.0, 2.0 * std::numbers::pi };
    std::bernoulli_distribution h_dist{ PROBABILITY_HEAVY };

    std::vector<std::array<task, CHUNK_SIZE>> chunks(CHUNK_COUNT);
    for (auto& chunk : chunks)
    {
        std::ranges::generate(
            chunk,
            [&]() -> task
            { return task{ .value = v_dist(rne), .heavy = h_dist(rne) }; });
    }
    return chunks;
}

inline auto generate_data_set_evenly()
    -> std::vector<std::array<task, CHUNK_SIZE>>
{
    std::minstd_rand rne;
    std::uniform_real_distribution v_dist{ 0.0, 2.0 * std::numbers::pi };

    std::vector<std::array<task, CHUNK_SIZE>> chunks(CHUNK_COUNT);
    for (auto& chunk : chunks)
    {
        std::ranges::generate(
            chunk,
            [&, acc = 0.0]() mutable -> task
            {
                bool heavy{};
                if (acc += PROBABILITY_HEAVY; acc >= 1.0)
                {
                    acc -= 1.0;
                    heavy = true;
                }
                return task{ .value = v_dist(rne), .heavy = heavy };
            });
    }
    return chunks;
}

inline auto generate_data_set_stacked()
    -> std::vector<std::array<task, CHUNK_SIZE>>
{
    auto chunks = generate_data_set_evenly();
    for (auto& chunk : chunks)
    {
        std::ranges::partition(chunk, std::identity{}, &task::heavy);
    }
    return chunks;
}

inline void process_dataset(std::span<const int> set, int& sum)
{
    for (const int data : set)
    {
        static constexpr auto limit =
            static_cast<double>(std::numeric_limits<int>::max());
        const double x = static_cast<double>(data) / limit;
        sum += static_cast<int>(sin(cos(x)) * limit);
    }
}
