#pragma once

#include "data.hpp"
#include <condition_variable>
#include <cstddef>
#include <mutex>

class master_control
{
public:
    explicit master_control()
        : m_lock(m_mtx)
    {
    }

    void signal_done()
    {
        bool needs_notification{};
        {
            const std::lock_guard m_lock(m_mtx);
            ++m_done_count;
            needs_notification = (m_done_count == WORKER_COUNT);
        }
        if (needs_notification) { m_cv.notify_one(); }
    }

    void wait_for_all_done()
    {
        m_cv.wait(m_lock, [this] { return m_done_count == WORKER_COUNT; });
        m_done_count = 0;
    }

private:
    std::condition_variable m_cv;
    std::mutex m_mtx;
    std::unique_lock<std::mutex> m_lock;

    // shared memory
    std::size_t m_done_count{};
};

[[maybe_unused]]
constexpr auto mctrl_size = sizeof(master_control);