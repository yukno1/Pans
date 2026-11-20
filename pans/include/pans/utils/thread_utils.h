#ifndef PANS_INCLUDE_PANS_UTILS_THREAD_UTILS_H
#define PANS_INCLUDE_PANS_UTILS_THREAD_UTILS_H

#include <pans/export.h>
#include <pans/macros.h>

#include <string>
#include <string_view>

namespace pans {
/**
 * @brief 获取线程ID
 *
 * 它在 Linux 下返回内核线程 ID，在Windows 下返回系统线程 ID；
 * 在其他平台返回 std::thread::id 的哈希值。
 */
[[nodiscard]] PANS_API u64 GetThreadId() noexcept;

/**
 * @brief 设置当前线程的名称。如果传入非法字符串，会读取当前线程本来的名字。
 */
PANS_API void SetThreadName(std::string name);

/**
 * @brief 获取当前线程的名称
 */
[[nodiscard]] PANS_API std::string_view GetThreadName() noexcept;

}  // namespace pans

#endif  // PANS_INCLUDE_PANS_UTILS_THREAD_UTILS_H
