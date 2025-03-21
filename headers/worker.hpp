#pragma once

#include "data.hpp"
#include "master_control.hpp"

#include <condition_variable>
#include <mutex>
#include <span>
#include <thread>

class worker
{
public:
    using value_type = int;

    explicit worker(master_control* master)
        : p_master(master), m_thread(&worker::run, this)
    {
    }

    void set_job(std::span<value_type> data, value_type* p_out)
    {
        {
            const std::lock_guard lk(m_mtx);
            m_input  = data;
            p_output = p_out;
        }
        m_cv.notify_one();
    }

    void kill()
    {
        {
            const std::lock_guard lk(m_mtx);
            m_dying = true;
        }
        m_cv.notify_one();
    }

private:
    void run()
    {
        std::unique_lock lk{ m_mtx };
        while (true)
        {
            m_cv.wait(lk, [this] { return p_output != nullptr || m_dying; });
            if (m_dying) { break; }
            process_dataset(m_input, *p_output);
            p_output = nullptr;
            m_input  = {};
            p_master->signal_done();
        }
    }

    master_control* p_master;
    std::jthread m_thread;
    std::condition_variable m_cv;
    std::mutex m_mtx;
    // shared memory
    std::span<value_type> m_input;
    value_type* p_output{};
    bool m_dying{};
};