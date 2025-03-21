#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>

class master_control
{
public:
    explicit master_control(std::size_t worker_count)
        : lk(mtx), worker_count(worker_count)
    {
    }

    void signal_done()
    {
        {
            const std::lock_guard lk(mtx);
            ++done_count;
        }
        if (done_count == worker_count) { cv.notify_one(); }
    }

    void wait_for_all_done()
    {
        cv.wait(lk, [this] { return done_count == worker_count; });
        done_count = 0;
    }

private:
    std::condition_variable cv;
    std::mutex mtx;
    std::unique_lock<std::mutex> lk;
    const std::size_t worker_count;
    // shared memory
    std::size_t done_count{};
};

[[maybe_unused]]
constexpr auto mctrl_size = sizeof(master_control);