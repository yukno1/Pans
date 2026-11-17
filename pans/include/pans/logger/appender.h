#ifndef PANS_INCLUDE_PANS_LOGGER_APPENDER_H
#define PANS_INCLUDE_PANS_LOGGER_APPENDER_H

#include <pans/export.h>
#include <pans/logger/log_level.h>

#include <memory>
#include <string>

namespace pans {

// 用来代理我们的内部权限，严格控制谁能访问我们的Appender私有实现
// 接口与实现隔离
namespace detail {
class AppenderAccess;
}

class PANS_API Appender final {
public:
    // 声明一个内部实现类, 类外定义
    class Impl;
    ~Appender() = default;

    Appender(const Appender&) = delete;             // 不希望被拷贝
    Appender& operator=(const Appender&) = delete;  // 不希望被移动构造
    Appender(Appender&&) = delete;                  // 不希望被移动复制
    Appender& operator=(Appender&&) = delete;

    void setLevel(LogLevel::Level level) noexcept;
    [[nodiscard]] LogLevel::Level getLevel() const noexcept;

    void flush();  // 把日志从进程缓冲区输出到os的缓冲区（脏页）
    void sync();   // 从os的脏页提交到磁盘的文件缓冲区

private:
    explicit Appender(std::unique_ptr<Impl> impl) noexcept;  // 由受控的工厂函数调用
    std::unique_ptr<Impl> m_impl;
    friend class detail::AppenderAccess;
};

using AppenderPtr = std::shared_ptr<Appender>;

[[nodiscard]] PANS_API AppenderPtr MakeStdoutAppender();
[[nodiscard]] PANS_API AppenderPtr MakeFileAppender(std::string file_name);

}  // namespace pans

#endif  // PANS_INCLUDE_PANS_LOGGER_APPENDER_H
