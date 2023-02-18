#pragma once

#include <spdlog/spdlog.h>
#include <filesystem>

#include "../log.hpp"

// std
template <>
struct fmt::formatter<std::filesystem::path> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const std::filesystem::path &path, FormatContext &ctx) const {
        return formatter<string_view>::format(path.string(), ctx);
    }
};
