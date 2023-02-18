#pragma once

#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_structs.h>
#include <entt/fwd.hpp>
#include "../log.hpp"
#include "../fmt/entt.hpp"

namespace physics {

template <typename T>
struct Shape {
    static constexpr auto in_place_delete = true;

    Shape(entt::entity e) : parent{e} {};
    Shape(const Shape &) = delete;
    Shape(Shape&& other) noexcept {
        memcpy(&shape, &other.shape, sizeof(shape));
        memset(&other.shape, 0, sizeof(shape));
        log_trace("this {}, other {}", fmt::ptr(this), fmt::ptr(&other));
    }
    ~Shape() {
        log_trace("this {}", fmt::ptr(this));
        cpShapeDestroy(&shape.shape);
    }

    inline operator cpShape*() { return &shape.shape; }
    inline operator const cpShape*() const { return &shape.shape; }

    entt::entity parent;
    T shape{};
};

struct Box : public Shape<cpPolyShape> {
    Box(cpFloat w, cpFloat h, cpFloat r, entt::entity e) : Shape(e) {
        log_trace("this {}, parent {}", fmt::ptr(this), e);
        cpBoxShapeInit(&shape, nullptr, w, h, r);
        //cpBoxShapeInit2(&shape, nullptr, cpBBNew(0, 0, w, h), r);
    };

    /// get the position of the top-left corner
    inline cpVect top_left() const {
        return shape.planes[3].v0;
    }
    /// get the size of the box
    inline cpVect size() const {
        return {
            shape.planes[0].v0.x - shape.planes[3].v0.x,
            shape.planes[1].v0.y - shape.planes[0].v0.y
        };
    }
};

struct Segment : public Shape<cpSegmentShape> {
    Segment(cpVect a, cpVect b, entt::entity e) : Shape(e) {
        log_trace("this {}", fmt::ptr(this));
        cpSegmentShapeInit(&shape, nullptr, a, b, 0);
    };

    /// get the body-relative position of point a
    inline cpVect a() const { return shape.a; }

    /// get the body-relative position of point b
    inline cpVect b() const { return shape.b; }

    /// get the space-relative position of point a
    inline cpVect ta() const { return shape.ta; }

    /// get the space-relative position of point b
    inline cpVect tb() const { return shape.tb; }
};

} // namespace physics
