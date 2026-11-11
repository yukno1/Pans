#include <pans/macros.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include "logger/buffer.h"
#include "logger/buffer_config.h"

/* 测试结果
operations: 100000, average of 7 runs

message: 50 characters
SmallStreamBuffer             4601838.43 ns     total, 46.02 ns/op
std::stringstream             10726616.57 ns    total, 107.27 ns/op
speedup: 2.33x

message: 100 characters
SmallStreamBuffer             4547746.29 ns     total, 45.48 ns/op
std::stringstream             10713650.86 ns    total, 107.14 ns/op
speedup: 2.36x

message: 200 characters
SmallStreamBuffer             5241695.86 ns     total, 52.42 ns/op
std::stringstream             10407049.86 ns    total, 104.07 ns/op
speedup: 1.99x

message: 500 characters
SmallStreamBuffer             5019006.14 ns     total, 50.19 ns/op
std::stringstream             10982399.71 ns    total, 109.82 ns/op
speedup: 2.19x

message: 1000 characters
SmallStreamBuffer             5255852.71 ns     total, 52.56 ns/op
std::stringstream             13926144.86 ns    total, 139.26 ns/op
speedup: 2.65x

message: 2048 characters
SmallStreamBuffer             7530506.00 ns     total, 75.31 ns/op
std::stringstream             17945912.14 ns    total, 179.46 ns/op
speedup: 2.38x
*/

constexpr int RUNS = 7;
u64 g_target = 0;
// 定义一个BENCHMARK_SINK变量，收集测量数据，让它产生一个编译器不可忽略的外部副作用，避免编译器把我们的逻辑优化掉
std::atomic<u64> BENCHMARK_SINK{0};

u64 Observe(std::string_view value) noexcept
{
    return static_cast<u64>(value.size()) +                       // 大小
           static_cast<unsigned char>(value.front()) +            // 第一个字节
           static_cast<unsigned char>(value[value.size() / 2]) +  // 中间字节
           static_cast<unsigned char>(value.back());
}

u64 WriteWithSmallStreamBuffer(std::string_view message)
{
    pans::detail::InlineBuffer<pans::detail::LOG_MESSAGE_INLINE_CAPACITY> buffer;
    pans::detail::SmallStreamBuffer<pans::detail::LOG_MESSAGE_INLINE_CAPACITY> stream_buffer(buffer);
    std::ostream stream(&stream_buffer);
    stream << message;
    return Observe(buffer.view());
}

u64 WriteWithStringStream(std::string_view message)
{
    std::stringstream stream;
    stream << message;
    return Observe(stream.view());
}

template <typename Operation>
std::chrono::nanoseconds BenchmarkWrites(Operation&& operation)
{
    u64 checksum = 0;
    const auto begin = std::chrono::steady_clock::now();
    for (u64 iteration = 0; iteration < g_target; ++iteration) {
        checksum += operation();
    }
    const auto end = std::chrono::steady_clock::now();
    BENCHMARK_SINK.fetch_xor(checksum, std::memory_order_relaxed);
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
}

template <typename Benchmark>
double Run(Benchmark&& benchmark)
{
    long double total_nanoseconds = 0.0L;
    for (int run = 0; run < RUNS; ++run) {
        total_nanoseconds += static_cast<long double>(benchmark().count());
    }
    return static_cast<double>(total_nanoseconds / RUNS);
}

void PrintResult(std::string_view name, double total_nanoseconds)
{
    std::cout << std::left << std::setw(30) << name << std::right << total_nanoseconds << " ns \ttotal, "
              << total_nanoseconds / static_cast<double>(g_target) << " ns/op\n";
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: ./test_log_buffer count\n";
        return 1;
    }
    g_target = std::stoull(argv[1]);  // 默认参数是正确的

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "operations: " << g_target << ", average of " << RUNS << " runs\n\n";
    constexpr std::array<std::size_t, 6> MESSAGE_SIZES = {50, 100, 200, 500, 1000, 2048};
    for (const std::size_t message_size : MESSAGE_SIZES) {
        const std::string message(message_size, 'x');
        const double small_stream_buffer_nanoseconds =
            Run([&]() { return BenchmarkWrites([&]() { return WriteWithSmallStreamBuffer(message); }); });
        const double stringstream_nanoseconds =
            Run([&]() { return BenchmarkWrites([&]() { return WriteWithStringStream(message); }); });

        std::cout << "message: " << message_size << " characters" << '\n';
        PrintResult("SmallStreamBuffer", small_stream_buffer_nanoseconds);
        PrintResult("std::stringstream", stringstream_nanoseconds);
        std::cout << "speedup: " << stringstream_nanoseconds / small_stream_buffer_nanoseconds << "x\n\n";
    }

    return 0;
}
