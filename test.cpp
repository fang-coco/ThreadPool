#include "threadpool.h"

using ulong = unsigned long long;

ulong sum(int a, int b) {
    ulong sum = 0;
    for (; a <= b; ++a) {
        sum += a;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return sum;
}

int main() {

    {
        ThreadPool threads;
        threads.setPoolMode(PoolMode::MODE_CACHED);
        threads.start(2);

        std::future<ulong> res1 = threads.submitTask(sum, 1, 100000000);
        std::future<ulong> res2 = threads.submitTask(sum, 2, 100000000);
        std::future<ulong> res3 = threads.submitTask(sum, 3, 100000000);
        std::future<ulong> res4 = threads.submitTask(sum, 4, 100000000);
        std::future<ulong> res5 = threads.submitTask(sum, 5, 100000000);
        std::future<ulong> res6 = threads.submitTask(sum, 6, 100000000);

        std::future<ulong> res11 = threads.submitTask(sum, 1, 100000000);
        std::future<ulong> res12 = threads.submitTask(sum, 2, 100000000);
        std::future<ulong> res13 = threads.submitTask(sum, 3, 100000000);
        std::future<ulong> res14 = threads.submitTask(sum, 4, 100000000);
        std::future<ulong> res15 = threads.submitTask(sum, 5, 100000000);
        std::future<ulong> res16 = threads.submitTask(sum, 6, 100000000);

        std::cout << res1.get() << std::endl;
        std::cout << res2.get() << std::endl;
        std::cout << res3.get() << std::endl;
        std::cout << res4.get() << std::endl;
        std::cout << res5.get() << std::endl;
        std::cout << res6.get() << std::endl;
        std::cout << res11.get() << std::endl;
        std::cout << res12.get() << std::endl;
        std::cout << res13.get() << std::endl;
        std::cout << res14.get() << std::endl;
        std::cout << res15.get() << std::endl;
        std::cout << res16.get() << std::endl;

    }

    return 0;
}