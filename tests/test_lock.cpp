#include <atomic>
// #include <charconv>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <latch>
#include <mutex>
// #include <numeric>
#include <shared_mutex>
// #include <stdexcept>
#include <pans/macros.h>

#include <string_view>
#include <thread>
#include <vector>

/*
operations: 100000, average of 7 runs

threads: 1
mutex         1275743.14 ns     total, 12.76 ns/op
shared mutex  2074182.00 ns     total, 20.74 ns/op
atomic        472636.29 ns      total, 4.73 ns/op
atomic CAS    798070.43 ns      total, 7.98 ns/op

threads: 2
mutex         3824039.57 ns     total, 38.24 ns/op
shared mutex  8685016.14 ns     total, 86.85 ns/op
atomic        1108403.57 ns     total, 11.08 ns/op
atomic CAS    1703408.14 ns     total, 17.03 ns/op

threads: 4
mutex         4342778.14 ns     total, 43.43 ns/op
shared mutex  8017352.14 ns     total, 80.17 ns/op
atomic        1166639.14 ns     total, 11.67 ns/op
atomic CAS    1983189.71 ns     total, 19.83 ns/op

threads: 8
mutex         4541158.71 ns     total, 45.41 ns/op
shared mutex  10621698.29 ns    total, 106.22 ns/op
atomic        1241831.71 ns     total, 12.42 ns/op
atomic CAS    2337357.71 ns     total, 23.37 ns/op

threads: 10
mutex         4839129.71 ns     total, 48.39 ns/op
shared mutex  12339138.00 ns    total, 123.39 ns/op
atomic        1284958.00 ns     total, 12.85 ns/op
atomic CAS    2506158.86 ns     total, 25.06 ns/op
*/

constexpr int RUNS = 7;  // 运行轮次

u64 g_target = 0;  // 执行次数
u64 g_Counter = 0;
std::atomic<u64> g_atomicCounter{0};
std::mutex g_mutex;
std::shared_mutex g_sharedMutex;  // 读写锁

u64 OperationsForThread(std::size_t thread_index, std::size_t thread_count) {
    const u64 base = g_target / thread_count;
    const u64 remainder = g_target % thread_count;
    return base + (thread_index < remainder ? 1 : 0);
}

// 跑多线程：将传递进来的模板函数用多个线程跑
template <typename Operation>
std::chrono::nanoseconds BenchmarkThreads(std::size_t thread_count, Operation&& operation) {
    // std::latch 是 c++20 加入的一次性线程同步计数器
    std::latch ready(static_cast<std::ptrdiff_t>(thread_count));
    std::latch start_gate(1);
    std::latch finished(static_cast<std::ptrdiff_t>(thread_count));
    // 通过 count_down 减 1
    // 在计数器归零前，所有的 wait 调用都会阻塞

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    for (std::size_t thread_index = 0; thread_index < thread_count; ++thread_index) {
        threads.emplace_back([&, thread_index]() {
            ready.count_down();
            start_gate.wait();
            operation(thread_index, OperationsForThread(thread_index, thread_count));
            finished.count_down();
        });
    }

    ready.wait();
    const auto begin = std::chrono::steady_clock::now();
    start_gate.count_down();
    finished.wait();
    const auto end = std::chrono::steady_clock::now();

    for (std::thread& thread : threads) {
        thread.join();
    }
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
}

std::chrono::nanoseconds BenchmarkMutex(std::size_t thread_count) {
    g_Counter = 0;
    const auto elapsed = BenchmarkThreads(thread_count, [](std::size_t, u64 operations) {
        for (u64 index = 0; index < operations; ++index) {
            std::lock_guard<std::mutex> lock(g_mutex);
            ++g_Counter;
        }
    });
    return elapsed;
}

std::chrono::nanoseconds BenchmarkSharedMutex(std::size_t thread_count) {
    g_Counter = 0;
    const auto elapsed = BenchmarkThreads(thread_count, [](std::size_t, u64 operations) {
        for (u64 index = 0; index < operations; ++index) {
            std::unique_lock<std::shared_mutex> lock(g_sharedMutex);
            ++g_Counter;
        }
    });
    return elapsed;
}

std::chrono::nanoseconds BenchmarkAtomic(std::size_t thread_count) {
    g_atomicCounter.store(0, std::memory_order_relaxed);
    const auto elapsed = BenchmarkThreads(thread_count, [](std::size_t, u64 operations) {
        for (u64 index = 0; index < operations; ++index) {
            g_atomicCounter.fetch_add(1, std::memory_order_relaxed);
        }
    });
    return elapsed;
}

std::chrono::nanoseconds BenchmarkAtomicCas(std::size_t thread_count) {
    g_atomicCounter.store(0, std::memory_order_relaxed);
    const auto elapsed = BenchmarkThreads(thread_count, [](std::size_t, u64 operations) {
        for (u64 index = 0; index < operations; ++index) {
            u64 expected = g_atomicCounter.load(std::memory_order_relaxed);
            while (!g_atomicCounter.compare_exchange_weak(
                expected, expected + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
            }
        }
    });
    return elapsed;
}

// 运行传递进来的函数
template <typename Benchmark>  // 万能引用
double Run(Benchmark&& benchmark) {
    long double total_nanoseconds = 0.0L;
    for (int run = 0; run < RUNS; ++run) {
        total_nanoseconds += static_cast<long double>(benchmark().count());
    }
    return static_cast<double>(total_nanoseconds / RUNS);
}

void PrintResult(std::string_view name, double total_nanoseconds) {
    const double nanoseconds_per_operation = total_nanoseconds / static_cast<double>(g_target);
    std::cout << std::left << std::setw(14) << name << std::right << total_nanoseconds << " ns \ttotal, "
              << nanoseconds_per_operation << " ns/op\n";
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./test_lock count\n";
        return -1;
    }
    g_target = std::atoi(argv[1]);

    std::cout << std::fixed << std::setprecision(2);  // 输出保留2位小数
    std::cout << "operations: " << g_target << ", average of " << RUNS << " runs\n\n";
    const std::vector<std::size_t> thread_counts = {1, 2, 4, 8, 10};
    for (const std::size_t thread_count : thread_counts) {
        std::cout << "threads: " << thread_count << '\n';
        PrintResult("mutex", Run([&]() { return BenchmarkMutex(thread_count); }));
        PrintResult("shared mutex", Run([&]() { return BenchmarkSharedMutex(thread_count); }));
        PrintResult("atomic", Run([&]() { return BenchmarkAtomic(thread_count); }));
        PrintResult("atomic CAS", Run([&]() { return BenchmarkAtomicCas(thread_count); }));
        std::cout << '\n';
    }

    return 0;
}
