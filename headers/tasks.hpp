#include "data.hpp"
#include "master_control.hpp"
#include "timer.hpp"
#include "worker.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
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

    const timer watch;

    std::vector<std::jthread> workers;
    workers.reserve(datasets.size());

    for (std::size_t i{}; i < datasets.size(); ++i)
    {
        workers.emplace_back(
            process_dataset,
            std::span{ datasets[i] },
            std::ref(sum[i].val));
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
    auto datasets = generate_data_set();
    std::array<value, DATASET_COUNT> sum_store{};

    const timer watch;

    const auto worker_cnt = DATASET_COUNT;
    master_control mctrl(worker_cnt);
    std::vector<std::unique_ptr<worker>> worker_ptrs;

    for (std::size_t i{}; i < worker_cnt; ++i)
    {
        worker_ptrs.push_back(std::make_unique<worker>(&mctrl));
    }

    static constexpr auto SUB_SIZE = DATASET_SIZE / 10'000;

    for (std::size_t j{}; j < DATASET_SIZE; j += SUB_SIZE)
    {
        for (std::size_t i{}; i < datasets.size(); ++i)
        {
            worker_ptrs[i]->set_job(
                std::span{ &datasets[i][j], SUB_SIZE },
                &sum_store[i].val);
        }
        mctrl.wait_for_all_done();
    }

    int result{};
    for (const auto& x : sum_store) { result += x.val; }
    const auto t = watch.elapsed();
    std::cout << "Processing the data sets took " << t << " seconds.\n";
    std::cout << "Result is " << result << '\n';

    for (auto& w : worker_ptrs) { w->kill(); }
    return 0;
}