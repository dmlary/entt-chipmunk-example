#include <chipmunk/chipmunk.h>
#include <entt/entt.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <util/sokol_gfx_imgui.h>
#include <util/sokol_imgui.h>

#include "../components.hpp"
#include "../entity_editor.hpp"
#include "../log.hpp"
#include "../physics.hpp"
#include "../physics/body.hpp"
#include "../physics/collision_type.hpp"
#include "../physics/shape.hpp"
#include "../render.hpp"
#include "../system.hpp"
#include "chipmunk/chipmunk_types.h"
#include "entt/core/algorithm.hpp"
#include "line_renderer.hpp"
#include "plugin.hpp"
#include "quad_renderer.hpp"

namespace render {

plugin::plugin(entt::registry&)
{
    log_debug("load render plugin");
    // setup sokol-gfx, sokol-time and sokol-imgui
    sg_desc desc = {};
    desc.context = sapp_sgcontext();
    sg_setup(&desc);
}

plugin::~plugin()
{
    log_debug("destroy render plugin");
    sg_shutdown();
}

entt::entity
plugin::create_camera(entt::registry& ecs)
{
    entt::entity cam = ecs.create();
    ecs.emplace<HumanDescription>(cam, "Camera: physics::Body",
        "Camera entity with associated physics::Body to control movement");
    ecs.emplace<Camera>(cam);

    // create a physics body for the camera, anchored in the top-left corner of
    // the screen.
    auto& body = ecs.emplace<physics::Body>(cam);
    cpBodySetPosition(body, { 0, 0 });
    ecs.emplace<physics::Movable>(cam, 40.0f, 200.0f);

    // set up a bounding box for the screen edges to detect when the player
    // reaches the edge
    const std::pair<cpVect, cpVect> bounds[]{
        {    { 0, 0 },   { 0, 176 }}, // left
        {  { 0, 176 }, { 320, 176 }}, // bottom
        {{ 320, 176 },   { 320, 0 }}, // right
        {  { 320, 0 },     { 0, 0 }}, // top
    };
    for (auto const& p : bounds) {
        auto shape = ecs.create();
        auto& line =
            ecs.emplace<physics::Segment>(shape, p.first, p.second, cam);
        cpShapeSetSensor(line, true);
        cpShapeSetCollisionType(line, physics::CT_Camera);
        ecs.emplace<HumanDescription>(shape, "Camera: ScreenEdgeSensor",
            "physics::Segment along the edge of the screen to detect"
            " entry/exit of physics bodies");
    }

    return cam;
}

void
plugin::move_camera(entt::registry& ecs, entt::entity camera, cpVect norm)
{
    // if the camera is already in motion, do nothing
    if (ecs.any_of<physics::Destination>(camera)) {
        return;
    }
    auto& body = ecs.get<physics::Body>(camera);

    cpVect pos = cpBodyGetPosition(body);
    ecs.emplace_or_replace<physics::Destination>(
        camera, pos + norm * RESOLUTION);

    ecs.emplace_or_replace<physics::Accelerate>(
        camera, physics::Accelerate{ norm * CAMERA_ACCEL, 400 });
}

void
plugin::reset_camera(entt::registry& ecs)
{
    auto& body = ecs.get<physics::Body>(m_camera);
    cpBodySetPosition(body, { 8, 8 });
}

void
plugin::init(entt::registry& ecs)
{
    log_debug("init graphics plugin");

    if (ecs.ctx().contains<entity_editor::plugin>()) {
        auto& editor = ecs.ctx().get<entity_editor::plugin>();
        editor.add<Translate>("render::Translate");
        editor.add<Sprite>("render::Sprite");
    }

    // create our renderers; we do this late to allow sokol_imgui to get it's
    // hooks in to be able to see any resources we allocate
    m_quad_renderer = std::make_unique<QuadRenderer>(40000, 60000);
    m_line_renderer = std::make_unique<LineRenderer>(40000, 60000);

    // create the camera
    m_camera = create_camera(ecs);

    // setup our systems
    entt::entity entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<Translate, const Sprite, const physics::Body>{
            .name  = "render::update_translate",
            .stage = System::Stage::draw - 10,
            .handler =
                [](auto&, auto& view, float) {
                    for (auto&& [e, tr, sprite, body] : view.each()) {
                        cpVect pos = body.pos();
                        tr.v.x = pos.x - sprite.res.x/2;
                        tr.v.y = pos.y - sprite.res.y/2;
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: update render::Translate",
        "copies position updates from physics::Body into"
        " render::Translate");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<const Camera, const physics::Collision>{
            .name  = "render::move_camera",
            .stage = System::Stage::draw - 10,
            .handler =
                [this](auto& ecs, auto& view, float) {
                    for (auto&& [e, col] : view.each()) {
                        move_camera(ecs, e, col.normal);
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: move_camera",
        "Moves camera when player collides with the screen boundary");

    // XXX does not need to happen every frame, only when Translate is updated,
    // or added
    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<>{
            .name  = "render::insert_sort_sprites",
            .stage = System::Stage::draw - 1,
            .handler =
                [](auto& ecs, float) {
                    entt::insertion_sort algo;
                    ecs.template sort<Translate>(
                        [](const auto& lhs, const auto& rhs) {
                            return lhs.z == rhs.z ? lhs.v.y < rhs.v.y
                                                  : lhs.z < rhs.z;
                        },
                        algo);
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: insert sort sprites",
        "insert sort sprites using standard sort");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<const Translate, const Sprite>{
            .name  = "render::draw_sprites",
            .always_run = true,
            .stage = System::Stage::draw,
            .handler =
                [this](auto&, auto& view, float) {
                    for (auto&& [e, tsl, sprite] : view.each()) {
                        m_quad_renderer->draw_rect(
                            tsl, sprite.res, sprite.img, sprite.crop);
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: draw render::Sprite",
        "performs draw calls for entities with render::Translate and"
        " render::Sprite");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<>{
            .name  = "render::draw_grid",
            .always_run = true,
            .stage = System::Stage::draw_debug - 1,
            .handler =
                [this](auto& ecs, float) {
                    auto &body = ecs.template get<physics::Body>(m_camera);
                    cpVect pos = body.pos();
                    cpVect tl  = pos - (pos % TILE_SIZE) + TILE_SIZE/2;
                    cpVect br  = pos + RESOLUTION;

                    for (int y = tl.y; y < br.y; y += TILE_SIZE.y) {
                        draw_line({ pos.x, y }, { br.x, y },
                            { 0x30, 0x60, 0x60, 0x80 });
                    }
                    for (int x = tl.x; x < br.x; x += TILE_SIZE.x) {
                        draw_line({ x, pos.y }, { x, br.y },
                            { 0x30, 0x60, 0x60, 0x80 });
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: draw grid",
            "draw tile grid on screen");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<>{
            .name  = "render::render",
            .stage = System::Stage::render,
            .always_run = true,
            .handler =
                [this](auto& ecs, float) {
                    // get the projection matrix from the camera physics body
                    auto& body     = ecs.template get<physics::Body>(m_camera);
                    cpVect pos     = cpBodyGetPosition(body);
                    glm::mat4 proj = glm::ortho(pos.x, pos.x + RESOLUTION.x,
                        pos.y, pos.y + RESOLUTION.y, -1.0f, 1.0f);

                    // start a pass
                    sg_pass_action pass = {
                        .colors[0].action = SG_ACTION_CLEAR,
                        .colors[0].value  = {0.0f, 0.5f, 0.7f, 1.0f},
                    };
                    sg_begin_default_pass(&pass, sapp_width(), sapp_height());

                    m_quad_renderer->render(proj);
                    m_line_renderer->render(proj);

                    sg_end_pass();
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: render",
        "renders all draw calls to the frame buffer");

    entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<>{
            .name  = "render::flush_frame",
            .stage = System::Stage::flush_frame,
            .always_run = true,
            .handler =
                [this](auto&, float) {
                    sg_commit();
                    m_quad_renderer->clear();
                    m_line_renderer->clear();
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: flush frame",
        "swaps the frame buffer, and resets all renderers for the next"
        " frame");
}

void
plugin::cleanup(entt::registry&)
{
    log_debug("cleanup graphics");
    m_quad_renderer.reset();
    m_line_renderer.reset();
    m_blank.reset();
}

void
plugin::draw_rect(vec2 pos, vec2 size, rgba color)
{
    m_line_renderer->draw_rect(pos, size, color);
}

void
plugin::draw_line(vec2 a, vec2 b, rgba color)
{
    m_line_renderer->draw_seg(a, b, color);
}
} // namespace render

namespace entity_editor {

template <>
void
draw_component<render::Translate>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<render::Translate>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("render::translate", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("v");
        ImGui::TableNextColumn();
        ImGui::DragScalarN("##pos", ImGuiDataType_Float, &obj.v.x, 2, 1.0f,
            nullptr, nullptr, "%0.3f");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("z");
        ImGui::TableNextColumn();
        ImGui::DragInt("##z", &obj.z);

        ImGui::EndTable();
    }
    ImGui::PopID();
}

} // namespace render
