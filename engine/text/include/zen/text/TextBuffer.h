#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace zen::text {

// Minimal contiguous text buffer.
// Placeholder — will be replaced with a piece-table / rope once the editor
// view is proven end-to-end.
class TextBuffer {
public:
    TextBuffer() = default;
    explicit TextBuffer(std::string initial);

    [[nodiscard]] std::string_view text() const noexcept { return m_data; }
    [[nodiscard]] std::size_t size() const noexcept { return m_data.size(); }
    [[nodiscard]] std::size_t lineCount() const noexcept;

    void setText(std::string text);
    void insert(std::size_t offset, std::string_view text);
    void erase(std::size_t offset, std::size_t length);

private:
    std::string m_data;
};

} // namespace zen::text
