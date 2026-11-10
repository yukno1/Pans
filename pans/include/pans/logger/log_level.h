#ifndef PANS_INCLUDE_PANS_LOGGER_LOG_LEVEL_H
#define PANS_INCLUDE_PANS_LOGGER_LOG_LEVEL_H

#include <pans/export.h>
#include <pans/types.h>

#include <string_view>  // c++17，如果不改变和拷贝，可以查看

namespace pans {
// 生产环境中，过于密集的日志会打爆磁盘；消耗性能；淹没重要信息
class PANS_API LogLevel final {
public:
    enum class Level : u8 {
        LOG_LV_DEBUG = 1,  // 记录调试细节，如变量值，程序执行流程
        LOG_LV_INFO = 2,   // 记录正常运行信息，比如服务启动，数据库连接成功，玩家连接成功
        LOG_LV_WARN = 3,   // 可能存在问题，但是程序还能继续执行
        LOG_LV_ERROR = 4,  // 操作失败，或者功能异常，需要检查和处理
        LOG_LV_FATAL = 5,  // 存在严重故障，程序继续执行，可能会有重大损失
        LOG_LV_OFF = 6,    // 用来关闭日志
    };

    [[nodiscard]] static std::string_view ToString(Level level) noexcept;              // 把int的level转成string
    [[nodiscard]] static LogLevel::Level FromString(std::string_view value) noexcept;  // 通过配置告诉系统
};

}  // namespace pans

#endif  // PANS_INCLUDE_PANS_LOGGER_LOG_LEVEL_H
