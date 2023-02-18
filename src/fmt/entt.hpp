#pragma once

#include <fmt/core.h>
#include <entt/entt.hpp>

#include "../log.hpp"

template <>
struct fmt::formatter<entt::entity> : fmt::formatter<uintptr_t> {
    template <typename FormatContext>
    auto format(entt::entity obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "{}", (uintptr_t)obj);
    }
};
