#pragma once

#include "data.hpp"
#include "master_control.hpp"
#include "timer.hpp"

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <thread>

class worker
{
public:
    using value_type = task;

    explicit worker(master_control* master)
        : p_master(master), m_thread(&worker::run, this)
    {
    }

    worker(const worker&)                        = delete;
    worker(const worker&&) noexcept              = delete;
    auto operator=(const worker&) -> worker&     = delete;
    auto operator=(worker&&) noexcept -> worker& = delete;

    virtual ~worker() { kill(); }

    void set_job(std::span<const value_type> data)
    {
        {
            const std::lock_guard lock(m_mtx);
            m_input = data;
        }
        m_cv.notify_one();
    }

    void kill()
    {
        {
            const std::lock_guard lock(m_mtx);
            m_dying = true;
        }
        m_cv.notify_one();
    }

    auto get_result() const -> std::uint32_t { return m_accumulation; }

    auto get_heavy_item_processed_cnt() const -> std::size_t
    {
        return m_heavy_items_prcessed;
    }

    auto get_job_work_time() const -> double { return m_work_time; }

private:
    void process_data()
    {
        m_heavy_items_prcessed = 0;
        for (auto& task : m_input)
        {
            m_accumulation += task.process();
            if constexpr (CHUNK_MEASUREMENT_ENABLED)
            {
                m_heavy_items_prcessed += (task.heavy) ? 1 : 0;
            }
        }
    }

    void run()
    {
        std::unique_lock lk{ m_mtx };
        while (true)
        {
            timer watch;
            m_cv.wait(lk, [this] { return !m_input.empty() || m_dying; });
            if (m_dying) { break; }
            if constexpr (CHUNK_MEASUREMENT_ENABLED) { watch.reset(); }
            process_data();
            if constexpr (CHUNK_MEASUREMENT_ENABLED)
            {
                m_work_time = watch.elapsed();
            }
            m_input = {};
            p_master->signal_done();
        }
    }

    master_control* p_master;
    std::jthread m_thread;
    std::condition_variable m_cv;
    std::mutex m_mtx;

    // shared memory
    std::span<const value_type> m_input;
    std::uint32_t m_accumulation{};
    double m_work_time = -1.0;
    std::size_t m_heavy_items_prcessed{};
    bool m_dying{};
};