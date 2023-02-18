#pragma once

#include <entt/fwd.hpp>
#include <chipmunk/chipmunk_types.h>
#include <glm/glm.hpp>
#include "render/plugin.hpp"
#include "physics.hpp"

namespace render {

constexpr cpVect RESOLUTION{320, 176};
constexpr cpVect CAMERA_ACCEL{RESOLUTION / 1.0};
constexpr cpVect TILE_SIZE{16, 16};

struct Camera {};

/// position of top-left corner of rendered entity
struct Translate {
    Translate(glm::vec2& p) : v{p} {};
    Translate(const cpVect& p) : v{p.x, p.y} {};
    Translate(float x, float y, int z) : v{x, y}, z{z} {};

    Translate& operator=(const cpVect& p) {
        v.x = p.x;
        v.y = p.y;
        return *this;
    }

    inline operator glm::vec2() { return v; };
    inline operator const glm::vec2() const { return v; };

    glm::vec2 v;
    int z{0};
};

/// sprite to render
struct Sprite {
    glm::vec2 res;  // resolution of sprite on screen
    sg_image img;   // image source for sprite
    glm::mat3 crop; // crop of img to render
};

} // namespace render
