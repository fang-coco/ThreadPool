#include "threadpool.h"

using ulong = unsigned long long;

class MyTask : public Task
{
public:
    MyTask(size_t start, size_t end)
        : m_start(start), m_end(end) {}

    // Task interface
public:
    virtual Any run() override
    {

        ulong sum = 0;
        for (int i = m_start; i <= m_end; ++i) {
            sum += i;
        }
//        std::this_thread::sleep_for(std::chrono::seconds(5));
        return sum;
    }
private:
    size_t m_start;
    size_t m_end;
};


int main() {

    {
        ThreadPool threads;
        threads.setPoolMode(PoolMode::MODE_CACHED);
        threads.start(4);

        auto res1 = threads.submitTask(std::make_shared<MyTask>(0, 10000000));
        auto res2 = threads.submitTask(std::make_shared<MyTask>(1, 10000000));
        auto res3 = threads.submitTask(std::make_shared<MyTask>(2, 10000000));
        auto res4 = threads.submitTask(std::make_shared<MyTask>(3, 10000000));
        auto res5 = threads.submitTask(std::make_shared<MyTask>(4, 10000000));
        auto res6 = threads.submitTask(std::make_shared<MyTask>(5, 10000000));

        auto res11 = threads.submitTask(std::make_shared<MyTask>(6, 10000000));
        auto res12 = threads.submitTask(std::make_shared<MyTask>(7, 10000000));
        auto res13 = threads.submitTask(std::make_shared<MyTask>(8, 10000000));
        auto res14 = threads.submitTask(std::make_shared<MyTask>(9, 10000000));
        auto res15 = threads.submitTask(std::make_shared<MyTask>(11, 10000000));
        auto res16 = threads.submitTask(std::make_shared<MyTask>(12, 10000000));

        std::cout <<  res1.get().cast<ulong>() << std::endl;
        std::cout <<  res2.get().cast<ulong>() << std::endl;
        std::cout <<  res3.get().cast<ulong>() << std::endl;
        std::cout <<  res4.get().cast<ulong>() << std::endl;
        std::cout <<  res5.get().cast<ulong>() << std::endl;
        std::cout <<  res6.get().cast<ulong>() << std::endl;

        std::cout <<  res11.get().cast<ulong>() << std::endl;
        std::cout <<  res12.get().cast<ulong>() << std::endl;
        std::cout <<  res13.get().cast<ulong>() << std::endl;
        std::cout <<  res14.get().cast<ulong>() << std::endl;
        std::cout <<  res15.get().cast<ulong>() << std::endl;
        std::cout <<  res16.get().cast<ulong>() << std::endl;
    }

    return 0;
}