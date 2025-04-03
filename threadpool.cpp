#include "threadpool.h"

const size_t MAX_TASK_NUMBERS =  4; //1 << 12;
const size_t MAX_THREAD_NUMBERS = 1 << 7;
const size_t MAX_IDEL_TIMEOUTS = 1;

ThreadPool::ThreadPool()
    : m_poolMode(PoolMode::MODE_FIXED)
    , m_running(false)
    , m_initThreads(0)
    , m_idelThreads(0)
    , m_currThreads(0)
    , m_maxThreads(MAX_THREAD_NUMBERS)
    , m_maxTasks(MAX_TASK_NUMBERS)
{}

ThreadPool::~ThreadPool()
{
    m_running = false;
    std::cout << "threadpool stopped!\n";
    std::unique_lock lock(m_mutex);
    m_emptyCond.notify_all();
    m_execCond.wait(lock, [&]() -> bool {
        return m_threads.size() == 0;
    });
}

void ThreadPool::start(size_t initThreads)
{
    if (m_running) return;
    m_running = true;
    m_initThreads = initThreads;
    m_idelThreads = m_initThreads;
    m_currThreads = m_initThreads;
    for (int i = 0; i < m_initThreads; ++i) {
        auto thread
            = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        m_threads.emplace(thread->threadID(), std::move(thread));
    }

    for (auto& thread: m_threads) {
        thread.second->start(); // 启动线程
    }
}

void ThreadPool::threadFunc(size_t threadID)
{
    auto lastTime = std::chrono::high_resolution_clock().now();
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock lock(m_mutex);
            while (m_taskQueue.empty()) {
                if (!m_running) {
                    m_threads.erase(threadID);
                    m_execCond.notify_all();
                    std::cout << threadID << " exit as threadpool was stopped!\n";
                    return;
                }
                if (checkModeIsCached()) {
                    if (std::cv_status::timeout ==
                        m_emptyCond.wait_for(lock, std::chrono::seconds(1))) {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                        if (dur.count() >= MAX_IDEL_TIMEOUTS
                         && m_currThreads > m_initThreads) {

                            m_threads.erase(threadID);
                            m_currThreads--;
                            m_idelThreads--;
                            std::cout << threadID << " thread had rebacked as timeout!\n";
                            return;
                        }
                    }
                } else {
                    std::cout << threadID << " has blocked as task queue is empty!\n";
                    m_emptyCond.wait(lock);
                }
            }
            m_idelThreads--;

            // 拿到队首的任务
            task = m_taskQueue.front();
            m_taskQueue.pop();

            if (!m_taskQueue.empty())
                m_emptyCond.notify_all();

            // 已经取出一个任务，表示当前队列not full，通知阻塞线程
            m_fullCond.notify_all();
        }
        std::cout << threadID << " take task and running!\n";
        if (task != nullptr)
            task();
        std::cout << threadID << " exec!\n";
        m_idelThreads++;
        lastTime = std::chrono::high_resolution_clock().now();
    }
}

bool ThreadPool::checkModeIsCached()
{
    return m_poolMode == PoolMode::MODE_CACHED;
}

void ThreadPool::creatThreads(size_t num)
{
    for (int i = 0; i < num; ++i) {
        auto thread
            = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        auto id = thread->threadID();
        m_threads.emplace(id, std::move(thread));
        m_threads[id]->start();
    }
    m_idelThreads += num;
}

void ThreadPool::setMaxThreads(size_t newMaxThreads)
{
    if (m_running) return;
    m_maxThreads = newMaxThreads;
}

void ThreadPool::setMaxTasks(size_t newMaxTasks)
{
    if (m_running) return;
    m_maxTasks = newMaxTasks;
}

void ThreadPool::setPoolMode(PoolMode newPoolMode)
{
    // 如果已经运行，不能设置mode
    if (m_running) return;
    m_poolMode = newPoolMode;
}

size_t Thread::g_threadID = 0;

Thread::Thread(FUNC func)
    : m_func(func)
    , m_threadID(g_threadID++)
{}

size_t Thread::threadID() const
{
    return m_threadID;
}

void Thread::start()
{
    // 创建线程，执行线程函数
    std::thread t(m_func, m_threadID);
    // 线程分离主线程，即使主线程结束，也不影响子线程
    t.detach();
}