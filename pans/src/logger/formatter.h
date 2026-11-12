#ifndef PANS_SRC_LOGGER_FORMATTER_H
#define PANS_SRC_LOGGER_FORMATTER_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "logger/buffer.h"
#include "logger/buffer_config.h"
#include "logger/log_record.h"

namespace pans::detail {

using FormattedRecordBuffer = InlineBuffer<FORMATTED_RECORD_INLINE_CAPACITY>;

class Formatter final {
public:
    // 所有 item 的父类，定义了 item 所有操作的接口
    // 面向接口编程和依赖倒置（其他：开闭原则，单一职责原则，接口隔离原则，依赖倒置原则，里氏替换原则，组合优于继承）
    class FormatItem {
    public:
        virtual ~FormatItem() = default;
        virtual void format(const LogRecordView& record, FormattedRecordBuffer& output) const = 0;
    };

    explicit Formatter(std::string_view pattern);
    // 根据 pattern 输出
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const;

    // 获取日志格式
    [[nodiscard]] const std::string& getPattern() const noexcept
    {
        return m_pattern;
    }

private:
    int parse();
    void addLiteral(std::string& literal);

private:
    std::string m_pattern;  // 一般通过外部传递
    std::vector<std::unique_ptr<FormatItem>> m_items;
};

}  // namespace pans::detail

#endif  // PANS_SRC_LOGGER_FORMATTER_H
