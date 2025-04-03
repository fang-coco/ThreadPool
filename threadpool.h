#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "ThreadPool_global.h"
#include <unordered_map>
#include <queue>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <future>


class Thread {
public:
    using FUNC = std::function<void(size_t)>;

    Thread(FUNC func);
    size_t threadID() const;
    void start();

private:
    FUNC m_func;
    static size_t g_threadID;

    size_t m_threadID;
};

enum class PoolMode {
    MODE_FIXED, // 固定线程数
    MODE_CACHED // 自动调整线程数
};

class THREADPOOL_EXPORT ThreadPool
{
public:
    ThreadPool();
    ~ThreadPool();

    void start(size_t initThreads = std::thread::hardware_concurrency());

    template <typename Func, typename... Args>
    auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
        using RType = decltype(func(args...));

        auto task = std::make_shared<std::packaged_task<RType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        std::future<RType> res = task->get_future();

        std::unique_lock lock(m_mutex);
            // 提交任务，超时时间1s
        if (!m_fullCond.wait_for(lock, std::chrono::seconds(1), [&]() -> bool {
                return m_taskQueue.size() <= m_maxTasks;
            })) {

            std::cerr << "task submit failed as taskqueue is fulled!\n";
            auto task = std::make_shared<std::packaged_task<RType()>>(
                    []()-> RType { return RType(); }
            );
            (*task)();
            return task->get_future();
        }

        // 提交成功，任务放入任务队列
        m_taskQueue.emplace([task]() -> void { // 这里必须按值传递，不然task对象被释放了
            (*task)();
        });

        // 任务队列中有任务了，通知阻塞在空队列的线程
        m_emptyCond.notify_all();

        if (m_poolMode == PoolMode::MODE_CACHED
            && m_taskQueue.size() > m_idelThreads) {
            size_t lastNums = m_currThreads;
            m_currThreads = std::min(m_maxThreads, m_currThreads << 1);
            creatThreads(m_currThreads - lastNums);
            std::cout << "creating threads, and current threads = " << m_currThreads << std::endl;
        }
        std::cout << "submit task...\n";
        return res;
    }

    void setPoolMode(PoolMode newPoolMode);

    void setMaxTasks(size_t newMaxTasks);

    void setMaxThreads(size_t newMaxThreads);

private:
    void threadFunc(size_t threadID);
    bool checkModeIsCached();

    void creatThreads(size_t num);

private:
    std::unordered_map<size_t, std::unique_ptr<Thread>> m_threads;
    std::queue<std::function<void()>> m_taskQueue;
    PoolMode m_poolMode;
    std::atomic_bool m_running;


    std::mutex m_mutex;
    std::condition_variable m_fullCond;
    std::condition_variable m_emptyCond;
    std::condition_variable m_execCond;

    size_t m_initThreads; // 初始化线程数
    std::atomic_int m_idelThreads; // 空闲线程数
    size_t m_currThreads; // 当前线程数
    size_t m_maxThreads;  // 最大线程数
    size_t m_maxTasks;    // 最大任务数
};

#endif // THREADPOOL_H
