#include <chipmunk/chipmunk.h>

#include "space.hpp"
#include "../log.hpp"

namespace physics {

Space::Space() {
    log_trace("this {}", fmt::ptr(this));
    cpSpaceInit(&m_space);
}

Space::Space(Space&& other) noexcept {
    log_trace("this {}, other {}", fmt::ptr(this), fmt::ptr(&other));
    memcpy(&m_space, &other.m_space, sizeof(m_space));
    memset(&other.m_space, 0, sizeof(m_space));
}

Space::~Space() {
    log_debug("this {}", fmt::ptr(this));
    // ugly way to see if the space has already been destroyed
    if (m_space.dynamicBodies != nullptr) {
        cpSpaceDestroy(&m_space);
    }
}

} // namespace physics
