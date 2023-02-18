#pragma once

#include <array>

namespace physics {

enum collision_type {
    CT_None = 0,
    CT_Player,
    CT_Camera,
    CT_Object,
    CT_Max,
};

extern const std::array<const char *, CT_Max> collision_type_name;

} // namespace physics
