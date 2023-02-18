#pragma once
#include <chipmunk/chipmunk.h>
#include <memory>

#include "entt/entity/fwd.hpp"
#include "physics/plugin.hpp"

static inline constexpr cpVect
operator+(const cpVect a, int v)
{
    return { a.x + v, a.y + v };
}

static inline constexpr cpVect
operator-(const cpVect a, int v)
{
    return { a.x - v, a.y - v };
}

static inline constexpr cpVect
operator*(const cpVect a, const cpVect b)
{
    return { a.x * b.x, a.y * b.y };
}

static inline constexpr cpVect
operator/(const cpVect a, const cpFloat b)
{
    return { a.x / b, a.y / b };
}

static inline constexpr cpVect
operator%(const cpVect a, cpFloat mod)
{
    return { fmod(a.x, mod), fmod(a.x, mod) };
}

static inline constexpr cpVect
operator%(const cpVect a, cpVect b)
{
    return { fmod(a.x, b.x), fmod(a.y, b.y) };
}

namespace physics {

class Space;
class Body;

struct Accelerate {
    cpVect force{};
    cpFloat cap;
};

struct Destination {
    cpVect pos{};
};

struct Movable {
    cpFloat accel;
    cpFloat velocity_max;
};

struct Collision {
    cpVect normal{};
};

/// cardinal directions
enum cardinal_direction {
    CD_Stopped = 0,
    CD_North,
    CD_South,
    CD_East,
    CD_West,
    CD_Other,
    CD_Max,
};

/// map from a cardinal direction to a vector for movement
extern const std::array<cpVect, CD_Max> cardinal_direction_vector;

/// move an entity along a cardinal direction
void move_entity_cardinal(entt::registry&, entt::entity, cardinal_direction);

/// get the cardinal direction for a vector
cardinal_direction vect_cardinal_direction(cpVect);

/// given a cardinal direction, return the reversed direction
cardinal_direction reverse_cardinal_direction(cardinal_direction dir);

/// given a vector, return the closest grid-aligned vector to that point
cpVect snap_to_grid(cpVect);

} // namespace physics
