#ifndef PANS_INCLUDE_PANS_MUTEX_H
#define PANS_INCLUDE_PANS_MUTEX_H

// 不同架构，为了cpu_relax()
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#elif defined(__aarch64__) || defined(__arm__)
#include <arm_acle.h>
#endif

#if defined(__linux__) && defined(__GLIBC__)
#include <pthread.h>
#else
#include <atomic>
#include <mutex>
#endif

inline void cpu_relax() noexcept
{
#if defined(__x86_64__) || defined(__i386__)
    // x86 专门为自旋等待设计的命令，告诉 cpu 线程正在忙等。
    // 能降低自旋循环的消耗；减少对cpu超线程资源的干扰；降低部分cpu退出循环的流水线开销。
    _mm_pause();
#elif defined(__aarch64__) || defined(__arm__)
    // __yield 类似
    __asm__ __volatile__("yield" ::: "memory");
#endif
}

namespace pans {

#if defined(__linux__) && defined(__GLIBC__)

class Spinlock {
public:
    Spinlock() noexcept
    {
        pthread_spin_init(&m_mutex, PTHREAD_PROCESS_PRIVATE);
    }

    ~Spinlock() noexcept
    {
        pthread_spin_destroy(&m_mutex);
    }

    Spinlock(const Spinlock&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;

    void lock() noexcept
    {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() noexcept
    {
        pthread_spin_unlock(&m_mutex);
    }

private:
    pthread_spinlock_t m_mutex{};
};

#else
// 标准平台，使用 atomic_flag
class Spinlock {
public:
    using Lock = std::lock_guard<Spinlock>;

    Spinlock() noexcept = default;
    ~Spinlock() noexcept = default;

    Spinlock(const Spinlock&) = delete;
    Spinlock& operator=(const Spinlock&) = delete;

    void lock() noexcept
    {
        // test_and_set 抢锁，将m_mutex 设为 true，返回原来的值
        while (m_mutex.test_and_set(std::memory_order_acquire)) {
            // 查看锁
            while (m_mutex.test(std::memory_order_relaxed)) {
                cpu_relax();
            }
        }
    }

    [[nodiscard]] bool try_lock() noexcept
    {
        return !m_mutex.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept
    {
        m_mutex.clear(std::memory_order_release);
    }

private:
    std::atomic_flag m_mutex = ATOMIC_FLAG_INIT;
};

#endif

}  // namespace pans

#endif  // PANS_INCLUDE_PANS_MUTEX_H
