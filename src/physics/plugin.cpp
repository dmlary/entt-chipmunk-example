#include <chipmunk/chipmunk.h>
#include <entt/fwd.hpp>
#include <imgui.h>

#include "../components.hpp"
#include "../entity_editor.hpp"
#include "../fmt/chipmunk.hpp"
#include "../fmt/entt.hpp"
#include "../imgui.hpp"
#include "../log.hpp"
#include "../physics.hpp"
#include "../system.hpp"
#include "../tags.hpp"
#include "body.hpp"
#include "chipmunk/chipmunk_unsafe.h"
#include "chipmunk/cpVect.h"
#include "collision_type.hpp"
#include "plugin.hpp"
#include "shape.hpp"
#include "space.hpp"

namespace physics {

static void
on_body_construct(entt::registry& ecs, entt::entity e)
{
    auto& body = ecs.get<Body>(e);
    log_trace("construct entity {}, Body {}", e, fmt::ptr(&body));
    auto& space = ecs.ctx().get<Space>();

    cpBodySetUserData(body, (void*)e);
    cpSpaceAddBody(space, body);
}

static void
on_body_destroy(entt::registry& ecs, entt::entity e)
{
    auto& body = ecs.get<Body>(e);
    log_trace("destroy entity {}, Body {}", e, fmt::ptr(&body));

    auto& space = ecs.ctx().get<Space>();
    cpSpaceRemoveBody(space, body);
}

template <typename Type>
static void
on_shape_construct(entt::registry& r, entt::entity e)
{
    auto& shape = r.get<Type>(e);
    log_trace("construct entity {}, shape {}", e, fmt::ptr(&shape));

    auto& body = r.get<Body>(shape.parent);
    cpShapeSetBody(shape, body);

    auto& space = r.ctx().get<Space>();
    cpSpaceAddShape(space, shape);
}

template <typename Type>
static void
on_shape_destroy(entt::registry& r, entt::entity e)
{
    auto& shape = r.get<Type>(e);
    log_trace("destroy entity {}, Shape {}", e, fmt::ptr(&shape));

    cpSpace* space = cpShapeGetSpace(shape);
    cpSpaceRemoveShape(space, shape);
}

plugin::plugin(entt::registry& ecs)
{
    log_debug("load physics plugin");

    ecs.on_construct<Body>().connect<on_body_construct>();
    ecs.on_destroy<Body>().connect<on_body_destroy>();

    ecs.on_construct<Box>().connect<on_shape_construct<Box>>();
    ecs.on_destroy<Box>().connect<on_shape_destroy<Box>>();

    ecs.on_construct<Segment>().connect<on_shape_construct<Segment>>();
    ecs.on_destroy<Segment>().connect<on_shape_destroy<Segment>>();
}

void
plugin::init(entt::registry& ecs)
{
    if (ecs.ctx().contains<entity_editor::plugin>()) {
        auto& editor = ecs.ctx().get<entity_editor::plugin>();
        editor.add<Space>("physics::Space");
        editor.add<Body>("physics::Body");
        editor.add<Box>("physics::Box");
        editor.add<Segment>("physics::Segment");
        editor.add<Movable>("physics::Movable");
        editor.add<Destination>("physics::Destination");
        editor.add<Accelerate>("physics::Accelerate");
        editor.add<Collision>("physics::Collision");
    }

    // set up physics space
    auto& space = ecs.ctx().emplace<Space>();
    cpSpaceSetGravity(space, { 0, 0 });
    cpSpaceSetUserData(space, &ecs);
    cpCollisionHandler* handler =
        cpSpaceAddWildcardHandler(space, physics::CT_Player);
    handler->userData  = &ecs;
    handler->beginFunc = [](cpArbiter* arb, cpSpace*,
                             cpDataPointer data) -> cpBool {
        auto* ecs = static_cast<entt::registry*>(data);
        assert(ecs != nullptr && "invalid ecs registry in space userdata");

        cpBody *a, *b;
        cpArbiterGetBodies(arb, &a, &b);
        // log_debug("collision player ({}), other ({})", *a, *b);

        entt::entity player = (entt::entity)(uintptr_t)cpBodyGetUserData(a);
        entt::entity other  = (entt::entity)(uintptr_t)cpBodyGetUserData(b);

        log_debug("collision: norm {}, depth {}, a {}, b {}",
            cpArbiterGetNormal(arb), cpArbiterGetDepth(arb, 1), player, other);

        cpVect normal = cpArbiterGetNormal(arb);
        ecs->emplace_or_replace<Collision>(other, normal);
        ecs->emplace_or_replace<Collision>(player, normal);

        return false;
    };

    entt::entity entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<Collision>{
            .name  = "physics::clear_collisions",
            .stage = System::Stage::update - 1,
            .handler =
                [](auto& ecs, auto& view, float) {
                    for (auto entity : view) {
                        ecs.template remove<Collision>(entity);
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "System: ClearCollisions",
        "clear collision component from all entities before physics update");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<>{
            .name    = "physics::step_space",
            .stage   = System::Stage::update,
            .handler = [&space](
                           auto&, float delta) { cpSpaceStep(space, delta); },
        });
    ecs.emplace<HumanDescription>(entity, "system: update physics",
        "Step the physics space to update all physics bodies");
    m_system_step = entity;

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<const Collision, Destination, Body>{
            .name  = "physics::collision_on_destination",
            .stage = System::Stage::update + 1,
            .handler =
                [](auto& ecs, auto& view, float) {
                    for (auto&& [entity, col, dest, body] : view.each()) {
                        body.stop();
                        dest.pos = snap_to_grid(body.pos());
                        ecs.template emplace_or_replace<Accelerate>(entity,
                                Accelerate{col.normal * -1 * 10000, 100});
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: update physics",
        "handle collisions for bodies with destinations");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<Box>{
            .name  = "physics::prune_orphaned_boxes",
            .stage = System::Stage::cleanup - 1,
            .handler =
                [](auto& ecs, auto& view, float) {
                    auto bodies = ecs.template view<Body>();
                    for (auto&& [entity, shape] : view.each()) {
                        if (!bodies.contains(shape.parent)) {
                            ecs.template emplace<tags::Destroy>(entity);
                        }
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "System: PruneBoxes",
        "prune any physics::Box entities whose parent entity no longer"
        " has exists, or no longer has a physics::Body");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<Segment>{
            .name  = "physics::prune_orphaned_segments",
            .stage = System::Stage::cleanup - 1,
            .handler =
                [](auto& ecs, auto& view, float) {
                    for (auto&& [entity, shape] : view.each()) {
                        if (!ecs.valid(shape.parent)) {
                            ecs.template emplace<tags::Destroy>(entity);
                        }
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: PruneSegments",
        "prune any physics::Segment entities whose parent entity no longer"
        " has exists, or no longer has a physics::Body");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<const Accelerate, Body>{
            .name  = "physics::accelerate_body",
            .stage = System::Stage::update - 1,
            .handler =
                [](auto& ecs, auto& view, float) {
                    for (auto&& [entity, accel, body] : view.each()) {
                        cpVect vel = body.velocity();
                        if (cpvlength(vel) >= accel.cap) {
                            cpBodySetVelocity(body, cpvclamp(vel, accel.cap));
                            ecs.template remove<Accelerate>(entity);
                        } else {
                            cpBodySetForce(body, accel.force);
                        }
                    }
                },
        });
    ecs.emplace<HumanDescription>(
        entity, "system: physics::accelerate", "accelerate physics bodies");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<const Destination, Body>{
            .name  = "physics::body_stopper",
            .stage = System::Stage::update + 1,
            .handler =
                [](auto& ecs, auto& view, float) {
                    for (auto&& [entity, dest, body] : view.each()) {
                        // log_debug("dest_system: e {}, body {}, dest {}",
                        //     entity, body, dest.pos);

                        cpVect v = body.velocity();
                        cpVect p = body.pos();
                        if (v.x == 0 && v.y == 0
                            && !ecs.template all_of<Accelerate>(entity)) {
                            log_debug("entity {} stranded at {}, dest {}",
                                entity, body.pos(), dest.pos);
                            ecs.template remove<Destination>(entity);
                            continue;
                        }

                        if (v.x == 0 && v.y == 0) {
                            continue;
                        }

                        if (v.x > 0 && p.x < dest.pos.x) {
                            continue;
                        } else if (v.x < 0 && p.x > dest.pos.x) {
                            continue;
                        }

                        if (v.y > 0 && p.y < dest.pos.y) {
                            continue;
                        } else if (v.y < 0 && p.y > dest.pos.y) {
                            continue;
                        }

                        // body at destination, let's halt at the right cords
                        cpBodySetVelocity(body, { 0, 0 });
                        cpBodySetPosition(body, dest.pos);
                        ecs.template remove<Destination>(entity);
                        if (ecs.template all_of<Accelerate>(entity)) {
                            ecs.template remove<Accelerate>(entity);
                        }
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: physics::destination",
        "stop physics bodies when they reach their destination");

}

} // namespace physics
