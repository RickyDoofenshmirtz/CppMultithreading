#pragma once

#include "data.hpp"
#include <array>
#include <chrono>
#include <cstddef>
#include <ratio>

class timer
{
public:
    using clock  = std::chrono::high_resolution_clock;
    using second = std::chrono::duration<double, std::ratio<1>>;

    timer()
        : m_begin(clock::now())
    {
    }

    void reset() { m_begin = clock::now(); }

    [[nodiscard]] auto elapsed() const -> double
    {
        return std::chrono::duration_cast<second>(clock::now() - m_begin)
            .count();
    }

private:
    std::chrono::time_point<clock> m_begin;
};

struct chunk_timing_info
{
    std::array<double, WORKER_COUNT> time_spent_working_per_thread{};
    std::array<std::size_t, WORKER_COUNT> number_of_heavy_itmes_per_thread{};
    double total_chunk_time{};
};