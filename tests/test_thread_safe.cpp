#include <atomic>
// #include <chrono>
#include <iostream>
// #include <mutex>
#include <thread>
#include <vector>

// int gs_your_money = 0;
std::atomic<int> gs_your_money = 0;
int thread_count = 10; // 线程数
int N = 100000;        // 每个线程执行的次数

// std::mutex gs_mutex;

void add_money() {
  for (int i = 0; i < N; ++i) {
    // std::lock_guard<std::mutex> lock(gs_mutex);
    ++gs_your_money;
  }
}
/*
如果没有锁，每次结果随机。
数据存在内存，但运算是在寄存器。先从内存读，在寄存器里+1，然后放回内存。
多线程不协调，读了相同的值。
互斥锁是保护临界区的一种手段。
原子变量：1/2/4/8字节的整型，编译器会生成cpu硬件原子指令，读改写在硬件层面不可分，性能极高。
*/

int main() {
  std::vector<std::thread> threads;
  for (int i = 0; i < thread_count; ++i) {
    threads.emplace_back(add_money);
  }
  for (auto &t : threads) {
    t.join();
  }
  std::cout << "------------------------------: " << gs_your_money << std::endl;
  return 0;
}
