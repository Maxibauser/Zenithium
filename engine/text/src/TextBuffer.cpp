#include "zen/text/TextBuffer.h"

#include <algorithm>
#include <utility>

namespace zen::text {

TextBuffer::TextBuffer(std::string initial) : m_data(std::move(initial)) {}

std::size_t TextBuffer::lineCount() const noexcept {
    if (m_data.empty()) {
        return 1;
    }
    const auto newlines = std::count(m_data.begin(), m_data.end(), '\n');
    return static_cast<std::size_t>(newlines) + (m_data.back() == '\n' ? 0 : 1);
}

void TextBuffer::setText(std::string text) {
    m_data = std::move(text);
}

void TextBuffer::insert(std::size_t offset, std::string_view text) {
    if (offset > m_data.size()) {
        offset = m_data.size();
    }
    m_data.insert(offset, text);
}

void TextBuffer::erase(std::size_t offset, std::size_t length) {
    if (offset >= m_data.size()) {
        return;
    }
    length = std::min(length, m_data.size() - offset);
    m_data.erase(offset, length);
}

} // namespace zen::text
