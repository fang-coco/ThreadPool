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

class Any {
public:
    Any() = default;
    ~Any() = default;

    Any(const Any&) = delete;
    Any& operator =(const Any&) = delete;

    Any(Any&&) = default;
    Any& operator =(Any&&) = default;

    template <typename T>
    Any(T data)
        : m_base(std::make_unique<Devire<T>>(data)) {}

    template <typename T>
    T cast() {
        auto ptr = dynamic_cast<Devire<T>*>(m_base.get());
        if(ptr == nullptr) {
            throw "type is unmatch!";
        }
        return ptr->m_data;
    }

private:
    class Base
    {
    public:
        Base() = default;
        virtual ~Base() = default;
    };

    template <typename T>
    class Devire : public Base
    {
    public:
        Devire(T data) : m_data(data) {}
        ~Devire() = default;
    public:
        T m_data;
    };

public:
    std::unique_ptr<Base> m_base;
};

class Semaphore
{
public:
    Semaphore(size_t resLimit = 0);

    void get();
    void post();
    void setValid(bool newValid);

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    size_t m_resLimit;
    bool m_valid;
};

class Task;

class Res {
public:
    Res(std::shared_ptr<Task> task, bool execed = false);
    ~Res() = default;

    Any get();
    void setAny(Any any);
private:
    std::atomic_bool m_execed;
    std::shared_ptr<Task> m_task;
    Semaphore m_semaphore;

    Any m_any;
};

class Task {
public:
    Task() = default;
    ~Task() {
        m_res = nullptr;
    };


    virtual Any run() = 0;
    void exec();
    void setRes(Res *newRes);

private:
    Res * m_res;
};

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
    Res submitTask(std::shared_ptr<Task> task);

    void setPoolMode(PoolMode newPoolMode);

    void setMaxTasks(size_t newMaxTasks);

    void setMaxThreads(size_t newMaxThreads);

private:
    void threadFunc(size_t threadID);
    bool checkModeIsCached();

    void creatThreads(size_t num);

private:
    std::unordered_map<size_t, std::unique_ptr<Thread>> m_threads;
    std::queue<std::shared_ptr<Task>> m_taskQueue;
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
