#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <random>
#include <span>
#include <vector>

static constexpr std::size_t DATASET_SIZE  = 50'000'000;
static constexpr std::size_t DATASET_COUNT = 4;

inline auto generate_data_set() -> std::vector<std::array<int, DATASET_SIZE>>
{
    const std::minstd_rand rne;
    std::vector<std::array<int, DATASET_SIZE>> datasets(DATASET_COUNT);
    for (auto& arr : datasets) { std::ranges::generate(arr, rne); }
    return datasets;
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
