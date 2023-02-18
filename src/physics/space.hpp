#pragma once

#include <chipmunk/chipmunk_structs.h>
#include "../log.hpp"

namespace physics {

class Space {
public:
    static constexpr auto in_place_delete = true;

    Space();
    Space(const Space &) = delete;
    Space(Space&& other) noexcept;
    ~Space();

    inline operator cpSpace*() { return &m_space; }
    inline operator const cpSpace*() const { return &m_space; }

private:
    cpSpace m_space{};
};

} // namespace physics
