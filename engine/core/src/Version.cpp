#include "zen/core/Version.h"

namespace zen::core {

Version version() noexcept {
    return {ZEN_VERSION_MAJOR, ZEN_VERSION_MINOR, ZEN_VERSION_PATCH};
}

std::string_view versionString() noexcept {
    return ZEN_VERSION_STRING;
}

} // namespace zen::core
