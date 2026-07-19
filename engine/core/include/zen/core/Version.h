#pragma once

#include <string_view>

namespace zen::core {

struct Version {
    int major;
    int minor;
    int patch;
};

Version version() noexcept;
std::string_view versionString() noexcept;

} // namespace zen::core
