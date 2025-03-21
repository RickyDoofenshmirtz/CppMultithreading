#pragma once

#include "data.hpp"
#include "timer.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <ostream>
#include <span>
#include <thread>
#include <vector>

struct value
{
    int val{};
    char padding[60]{};
};

inline auto do_biggie() -> std::int32_t
{
    auto datasets = generate_data_set();
    std::array<value, DATASET_COUNT> sum{};

    std::vector<std::jthread> workers;
    workers.reserve(datasets.size());

    const timer watch;
    for (std::size_t i{}; i < datasets.size(); ++i)
    {
        workers.emplace_back(
            process_dataset, std::span{ datasets[i] }, std::ref(sum[i].val));
    }
    workers.clear();
    int result{};
    for (const auto& x : sum) { result += x.val; }
    const auto t = watch.elapsed();
    std::cout << "Processing the data sets took " << t << " seconds.\n";
    std::cout << "Result is " << result << '\n';
    return 0;
}

inline auto do_smallies() -> std::int32_t
{
    timer watch;
    auto datasets = generate_data_set();
    std::array<value, DATASET_COUNT> sum_store{};

    std::vector<std::jthread> workers;
    workers.reserve(datasets.size());

    static constexpr auto SUB_SIZE = DATASET_SIZE / 100;

    watch.reset();
    for (std::size_t j{}; j < DATASET_SIZE; j += SUB_SIZE)
    {
        for (std::size_t i{}; i < datasets.size(); ++i)
        {
            workers.emplace_back(
                process_dataset, std::span{ &datasets[i][j], SUB_SIZE },
                std::ref(sum_store[i].val));
        }
        workers.clear();
    }
    int result{};
    for (const auto& x : sum_store) { result += x.val; }
    const auto t = watch.elapsed();
    std::cout << "Processing the data sets took " << t << " seconds.\n";
    std::cout << "Result is " << result << '\n';
    return 0;
}