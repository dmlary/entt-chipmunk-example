#pragma once

#include <chipmunk/chipmunk.h>

#include "../log.hpp"

// chipmunk2d
template <>
struct fmt::formatter<cpVect> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(cpVect obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "({:.3f}, {:.3f})",
                obj.x, obj.y);
    }
};

template <>
struct fmt::formatter<cpBody> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const cpBody& obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(), "p {}, v {}",
                cpBodyGetPosition(&obj), cpBodyGetVelocity(&obj));
    }
};
