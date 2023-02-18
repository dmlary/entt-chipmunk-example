#include "debug_draw.hpp"
#include "../components.hpp"
#include "../imgui.hpp"
#include "../render.hpp"
#include "../system.hpp"
#include "body.hpp"
#include "shape.hpp"

namespace physics {

void
debug_draw::init(entt::registry& ecs)
{
    m_system = ecs.create();
    ecs.emplace<HumanDescription>(m_system, "system: physics draw debug",
        "draw debug shapes for physics components");
    ecs.emplace<System>(m_system,
        System::Config<>{
            .name       = "physics::debug_draw",
            .always_run = true,
            .stage      = System::Stage::draw_debug,
            .handler    = [this](auto& ecs, float) { draw(ecs); },
        });
}

void
debug_draw::cleanup(entt::registry& ecs)
{
    ecs.destroy(m_system);
}

void
debug_draw::draw(entt::registry& ecs)
{
    auto& render = ecs.ctx().template get<render::plugin>();

    if (m_draw_segment) {
        for (auto&& [entity, segment] : ecs.view<const Segment>().each()) {
            cpVect a = segment.ta();
            cpVect b = segment.tb();

            render.draw_line(
                { a.x, a.y }, { b.x, b.y }, { 0xff, 0, 0xff, 0xff });
        }
    }

    if (m_draw_box) {
        for (auto&& [entity, box] : ecs.view<const Box>().each()) {
            cpVect pos  = box.top_left();
            cpVect size = box.size();

            render.draw_rect(
                { pos.x, pos.y }, { size.x, size.y }, { 0xff, 0, 0, 0xff });
        }
    }

    if (m_draw_body) {
        for (auto&& [entity, body] : ecs.view<const Body>().each()) {
            cpVect pos = body.pos();
            render.draw_rect(
                { pos.x - 1, pos.y - 1 }, { 2, 2 }, { 0xff, 0xff, 0, 0xff });
        }
    }

    if (m_draw_dest) {
        for (auto&& [entity, body, dest] :
                ecs.view<const Body, const Destination>().each()) {
            cpVect a = body.pos();
            cpVect b = dest.pos;

            render.draw_line(
                { a.x, a.y }, { b.x, b.y }, { 0xff, 0xff, 0, 0xff });
        }
    }
}

} // namespace render
