#pragma once

#include <thread>
#include <condition_variable>
#include <deque>
#include <chrono>
#include "bulk.h"

template <typename T>
class assyncPrinter: public printer
{
public:
    assyncPrinter(std::size_t threadsCount = 1){
        if(threadsCount == 0)
            threadsCount = 1;
        while (threadsCount > 0)
        {
            m_theads.emplace_back([this, prin = std::make_unique<T>()](std::stop_token stop_token) mutable
            {
                while(!stop_token.stop_requested())
                {
                    std::unique_lock lk(m_mutex);
                    m_cv.wait(lk, [this, &stop_token]
                    {
                        return stop_token.stop_requested() || (m_bulkq.size() > 0);
                    });
                
                    if(!m_bulkq.empty())
                    {
                        prin->printBulk(m_bulkq.front());
                        m_bulkq.pop_front();
                    }
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1ms);
                }

                std::lock_guard m{m_mutex};
                while (!m_bulkq.empty())
                {
                    prin->printBulk(m_bulkq.front());
                        m_bulkq.pop_front();
                }
                
            });
            --threadsCount;
        }
    }
    ~assyncPrinter()
    {
        for(auto &&t: m_theads)
            t.request_stop();
        m_cv.notify_all();
    }

    void printBulk(std::shared_ptr<bulk> curBulk) override final
    {
        std::lock_guard m{m_mutex};
        m_bulkq.push_back(curBulk);
        m_cv.notify_all();
    }
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::deque<std::shared_ptr<bulk>> m_bulkq;
    std::vector<std::jthread> m_theads;
};
