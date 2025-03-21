#pragma once

#include <chrono>
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
