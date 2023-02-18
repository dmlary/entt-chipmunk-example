#pragma once

#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_structs.h>
#include <entt/fwd.hpp>

#include "../log.hpp"
#include "../fmt/chipmunk.hpp"
#include "../physics.hpp"
#include "chipmunk/cpVect.h"
#include "space.hpp"

namespace physics {

class Body {
public:
    static constexpr auto in_place_delete = true; // NOLINT

    Body();
    Body(const Body&) = delete;
    Body(Body&& other) noexcept;
    ~Body();

    Body& operator=(const Body&) = delete;
    Body& operator=(Body&&) = delete;

    inline operator cpBody*() { return &m_body; }
    inline operator const cpBody*() const { return &m_body; }
    inline cpVect pos() const { return cpBodyGetPosition(&m_body); }
    inline cpVect cog() const { return cpBodyGetCenterOfGravity(&m_body); }
    inline cpVect velocity() const { return cpBodyGetVelocity(&m_body); }
    inline bool stopped() const {
        cpVect v = velocity();
        return cpvnear(v, {0,0}, 0.01);
    }
    inline void stop() {
        cpBodySetVelocity(&m_body, {0,0});
    }

    cardinal_direction cardinal_direction() const;

private:
    cpBody m_body {};
};

} // namespace physics

template <>
struct fmt::formatter<physics::Body> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const physics::Body& obj, FormatContext& ctx) const
    {
        return fmt::format_to(ctx.out(), "pos {}, v {}",
            cpBodyGetPosition(obj), cpBodyGetVelocity(obj));
    }
};
