#pragma once

#include "data.hpp"
#include "master_control.hpp"
#include "timer.hpp"
#include "worker.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include <span>
#include <vector>

inline void write_to_csv(std::span<const chunk_timing_info> timings)
{
    std::ofstream csv("timings.csv", std::ios_base::trunc);

    for (std::size_t i{}; i < WORKER_COUNT; ++i)
    {
        csv << std::format("work_{0:},idle{0:},heavy{0:},", i);
    }
    csv << "chunk_time,total_idle,total_heavy\n";

    for (const auto& chunk : timings)
    {
        double total_idle{};
        std::size_t total_heavy{};
        for (std::size_t i{}; i < WORKER_COUNT; ++i)
        {
            const auto idle  = chunk.total_chunk_time - chunk.time_spent_working_per_thread[i];
            const auto heavy = chunk.number_of_heavy_itmes_per_thread[i];
            csv << std::format("{},{},{},", chunk.time_spent_working_per_thread[i], idle, heavy);
            total_idle += idle;
            total_heavy += heavy;
        }
        csv << std::format("{},{},{}\n", chunk.total_chunk_time, total_idle, total_heavy);
    }
}

inline auto do_experiment(bool stacked) -> int
{
    const auto chunks = (stacked) ? generate_data_set_stacked() : generate_data_set_evenly();

    timer chunk_timer;
    std::vector<chunk_timing_info> timings{};
    timings.reserve(CHUNK_COUNT);

    const timer total_timer;

    master_control mctrl;
    std::vector<std::unique_ptr<worker>> worker_ptrs(WORKER_COUNT);

    std::ranges::generate(
        worker_ptrs,
        [p_mctrl = &mctrl] { return std::make_unique<worker>(p_mctrl); });

    for (auto& chunk : chunks)
    {
        if constexpr (CHUNK_MEASUREMENT_ENABLED) { chunk_timer.reset(); }
        for (std::size_t i{}; i < WORKER_COUNT; ++i)
        {
            worker_ptrs[i]->set_job(std::span{ &chunk[i * SUBSET_SIZE], SUBSET_SIZE });
        }
        mctrl.wait_for_all_done();

        if constexpr (CHUNK_MEASUREMENT_ENABLED)
        {
            timings.push_back(chunk_timing_info{ .total_chunk_time = chunk_timer.elapsed() });
            for (std::size_t i{}; i < WORKER_COUNT; ++i)
            {
                auto& curr = timings.back();
                curr.number_of_heavy_itmes_per_thread[i] =
                    worker_ptrs[i]->get_heavy_item_processed_cnt();
                curr.time_spent_working_per_thread[i] = worker_ptrs[i]->get_job_work_time();
            }
        }
    }
    const auto time = total_timer.elapsed();
    std::cout << "Processing took " << time << " seconds\n";

    std::uint32_t final_result{};
    for (const auto& w : worker_ptrs) { final_result += w->get_result(); }
    std::cout << "result is " << final_result << '\n';

    if constexpr (CHUNK_MEASUREMENT_ENABLED) { write_to_csv(timings); }

    return 0;
}