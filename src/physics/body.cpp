#include "body.hpp"
#include "../physics.hpp"

namespace physics {

Body::Body() {
    log_trace("this {}", fmt::ptr(this));
    cpBodyInit(&m_body, 1, INFINITY);
}

Body::Body(Body&& other) noexcept {
    log_trace("this {}, other {}", fmt::ptr(this), fmt::ptr(&other));
    memcpy(&m_body, &other.m_body, sizeof(m_body));
    memset(&other.m_body, 0, sizeof(m_body));
}

Body::~Body() {
    log_trace("this {}", fmt::ptr(this));
    cpBodyDestroy(&m_body);
}

cardinal_direction
Body::cardinal_direction() const
{
    cpVect v = velocity();
    if (v.x == 0 && v.y == 0) {
        return CD_Stopped;
    }
    if (v.x != 0 && v.y != 0) {
        return CD_Other;
    }

    if (v.y < 0) {
        return CD_North;
    } else if (v.y > 0) {
        return CD_South;
    }

    if (v.x < 0) {
        return CD_West;
    } else if (v.x > 0) {
        return CD_East;
    }

    assert(false && "invalid case");
}

} // namespace physics
