#pragma once

#include <glm/glm.hpp>

#include "../log.hpp"

template <>
struct fmt::formatter<glm::vec2> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(glm::vec2 obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "{{ .x = {}, .y = {} }}",
                obj.x, obj.y);
    }
};

template <>
struct fmt::formatter<glm::vec3> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(glm::vec3 obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "{{ .x = {}, .y = {}, .z = {} }}",
                obj.x, obj.y, obj.z);
    }
};

template <>
struct fmt::formatter<glm::vec4> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(glm::vec4 obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "{{ .x = {}, .y = {}, .z = {}, .w = {} }}",
                obj.x, obj.y, obj.z, obj.q);
    }
};
template <>
struct fmt::formatter<glm::vec<4, uint8_t>> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(glm::vec<4, uint8_t> obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "{{ {}, {}, {}, {} }}",
                obj.r, obj.g, obj.b, obj.a);
    }
};

template <>
struct fmt::formatter<glm::mat3> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(glm::mat3 obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "\n\t({}, {}, {}),"
                "\n\t({}, {}, {}),"
                "\n\t({}, {}, {})",
                obj[0][0], obj[1][0], obj[2][0],
                obj[0][1], obj[1][1], obj[2][1],
                obj[0][2], obj[1][2], obj[2][2]);
    }
};
template <>
struct fmt::formatter<glm::mat4> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(glm::mat4 obj, FormatContext &ctx) const {
        return fmt::format_to(ctx.out(),
                "\n\t({}, {}, {}, {}),"
                "\n\t({}, {}, {}, {}),"
                "\n\t({}, {}, {}, {}),"
                "\n\t({}, {}, {}, {})",
                obj[0][0], obj[1][0], obj[2][0], obj[3][0],
                obj[0][1], obj[1][1], obj[2][1], obj[3][1],
                obj[0][2], obj[1][2], obj[2][2], obj[3][2],
                obj[0][3], obj[1][3], obj[2][3], obj[3][3]);
    }
};
