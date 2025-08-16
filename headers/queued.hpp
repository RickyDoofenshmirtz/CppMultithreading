#pragma once

#include "data.hpp"
#include "timer.hpp"

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <span>
#include <thread>

namespace que {
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

        void set_chunk(std::span<const task> chunk)
        {
            m_idx           = 0;
            m_current_chunk = chunk;
        }

        auto get_task() noexcept -> const task*
        {
            const std::lock_guard<std::mutex> lock(m_mtx);
            const auto i = m_idx++;
            if (i >= CHUNK_SIZE) { return {}; }
            return &m_current_chunk[i];
        }

    private:
        std::condition_variable m_cv;
        std::mutex m_mtx;
        std::unique_lock<std::mutex> m_lock;
        std::span<const task> m_current_chunk;

        // shared memory
        std::size_t m_done_count{};
        std::size_t m_idx{};
    };

    class worker
    {
    public:
        using value_type = task;

        explicit worker(master_control* master)
            : m_master(master), m_thread(&worker::_run, this)
        {
        }

        worker(const worker&)                        = delete;
        worker(const worker&&) noexcept              = delete;
        auto operator=(const worker&) -> worker&     = delete;
        auto operator=(worker&&) noexcept -> worker& = delete;

        virtual ~worker() { kill(); }

        void start_work()
        {
            {
                const std::lock_guard lock(m_mtx);
                m_working = true;
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

        auto get_heavy_item_processed_cnt() const -> std::size_t { return m_heavy_items_prcessed; }

        auto get_job_work_time() const -> double { return m_work_time; }

    private:
        void _run()
        {
            std::unique_lock lk{ m_mtx };
            while (true)
            {
                timer watch;
                m_cv.wait(lk, [this] { return m_working || m_dying; });
                if (m_dying) { break; }
                if constexpr (CHUNK_MEASUREMENT_ENABLED) { watch.reset(); }
                _process_data();
                if constexpr (CHUNK_MEASUREMENT_ENABLED) { m_work_time = watch.elapsed(); }
                m_working = false;
                m_master->signal_done();
            }
        }

        void _process_data()
        {
            m_heavy_items_prcessed = 0;
            while (auto ptask = m_master->get_task())
            {
                m_accumulation += ptask->process();
                if constexpr (CHUNK_MEASUREMENT_ENABLED)
                {
                    m_heavy_items_prcessed += (ptask->heavy) ? 1 : 0;
                }
            }
        }

        master_control* m_master;
        std::jthread m_thread;
        std::condition_variable m_cv;
        std::mutex m_mtx;

        // shared memory
        std::uint32_t m_accumulation{};
        bool m_dying{};
        bool m_working{};

        double m_work_time = -1.0;
        std::size_t m_heavy_items_prcessed{};
    };
} // namespace que
