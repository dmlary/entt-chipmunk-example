#include <chipmunk/chipmunk.h>
#include <chipmunk/chipmunk_structs.h>
#include <chipmunk/chipmunk_unsafe.h>
#include <entt/entt.hpp>
#include <imgui.h>

#include "entity_editor.hpp"
#include "log.hpp"
#include "render.hpp"
#include "physics.hpp"
#include "physics/body.hpp"
#include "physics/collision_type.hpp"
#include "physics/shape.hpp"

namespace physics {

const std::array<cpVect, CD_Max> cardinal_direction_vector = {
    cpVect{ 0, 0 }, // stopped
    cpVect{ 0, -1 }, // north
    cpVect{ 0, 1 }, // south
    cpVect{ 1, 0 }, // east
    cpVect{ -1, 0 }, // west
    cpVect{ 0, 0 } // other
};

void
move_entity_cardinal(entt::registry& ecs,
    entt::entity entity,
    cardinal_direction dir)
{
    Body& body = ecs.get<Body>(entity);
    if (!body.stopped()) {
        return;
    }

    cpVect pos  = body.pos();
    pos = pos - (pos % render::TILE_SIZE);
    cpVect vect = cardinal_direction_vector[dir] * render::TILE_SIZE;
    cpVect dest = pos + vect;
    log_debug("pos {}, adj {}, mod {}, dest {}",
            body.pos(), pos, pos % render::TILE_SIZE, dest);

    ecs.emplace_or_replace<Destination>(entity, dest);

    Movable& m = ecs.get<Movable>(entity);
    ecs.emplace_or_replace<Accelerate>(
        entity, Accelerate{ vect * m.accel, m.velocity_max });
}

cardinal_direction
vect_cardinal_direction(cpVect v) {
    if (v.x == 0 && v.y == 0) {
        return CD_Stopped;
    }
    if (v.x != 0 && v.y != 0) {
        return CD_Other;
    }
    if (v.x > 0) {
        return CD_East;
    }
    if (v.x < 0) {
        return CD_West;
    }
    if (v.y > 0) {
        return CD_South;
    }
    if (v.y < 0) {
        return CD_North;
    }
    assert(true && "we shouldn't get here");
    return CD_Other;
}

cardinal_direction
reverse_cardinal_direction(cardinal_direction dir)
{
    switch (dir) {
    case CD_Stopped: return CD_Stopped;
    case CD_North: return CD_North;
    case CD_South: return CD_North;
    case CD_East: return CD_West;
    case CD_West: return CD_East;
    default: return CD_Other;
    }
}

cpVect
snap_to_grid(cpVect pos)
{
    cpVect dist = pos % render::TILE_SIZE;
    cpVect tr = pos - dist;
    return {
        dist.x <= (render::TILE_SIZE.x / 2) ? tr.x : tr.x + render::TILE_SIZE.x,
        dist.y <= (render::TILE_SIZE.y / 2) ? tr.y : tr.y + render::TILE_SIZE.y
    };
}

} // namespace physics

namespace entity_editor {

using namespace physics;

template <>
void
draw_component<Body>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<Body>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("physics::body", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Position");
            ImGui::TableNextColumn();
            cpVect orig = cpBodyGetPosition(obj);
            cpVect pos  = orig;
            ImGui::DragScalarN("##pos", ImGuiDataType_Float, &pos.x, 2, 1.0f,
                nullptr, nullptr, "%0.3f");
            if (pos.x != orig.x || pos.y != orig.y) {
                cpBodySetPosition(obj, pos);
            }
        }

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Velocity");
            ImGui::TableNextColumn();

            cpVect orig = cpBodyGetVelocity(obj);
            cpVect vel  = orig;

            ImGui::DragScalarN("##vel", ImGuiDataType_Float, &vel.x, 2, 0.1f,
                nullptr, nullptr, "%0.3f");
            ImGui::SameLine();

            if (ImGui::Button("stop")) {
                vel = { 0, 0 };
            }
            if (vel.x != orig.x || vel.y != orig.y) {
                cpBodySetVelocity(obj, vel);
            }
        }

        ImGui::EndTable();
    }
    ImGui::PopID();
}

template <>
void
draw_component<Box>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<Box>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("physics::box", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 100.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Body Entity:");
            ImGui::TableNextColumn();
            ImGui::InputScalar("##parent", ImGuiDataType_U32, &obj.parent,
                nullptr, nullptr, "%d");
            ImGui::SameLine();
            if (ImGui::Button("Open")) {
                auto& e = ecs.ctx().get<entity_editor::plugin>();
                e.create_editor(ecs, obj.parent);
            }
        }
        {
            unsigned type = cpShapeGetCollisionType(obj);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Collision Type:");
            ImGui::TableNextColumn();
            if (ImGui::BeginCombo(
                    "##collision_type", collision_type_name[type])) {
                for (size_t i = 0; i < collision_type_name.size(); i++) {
                    const bool selected = (type == i);
                    if (ImGui::Selectable(collision_type_name[i], selected)) {
                        cpShapeSetCollisionType(obj, i);
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Sensor:");
            ImGui::TableNextColumn();
            ImGui::Checkbox("##sensor", (bool*)&obj.shape.shape.sensor);
        }

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Radius:");
            ImGui::TableNextColumn();
            ImGui::InputScalar("##radius", ImGuiDataType_Float, &obj.shape.r);
        }

        float width = obj.shape.planes[0].v0.x - obj.shape.planes[3].v0.x;
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Width:");
        ImGui::TableNextColumn();
        ImGui::Text("%0.0f", width);

        float height = obj.shape.planes[1].v0.y - obj.shape.planes[0].v0.y;
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("height:");
        ImGui::TableNextColumn();
        ImGui::Text("%0.0f", height);

        // calculate the width & height from the planes
        for (int i = 0; i < obj.shape.count; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("plane[%d]:", i);
            ImGui::TableNextColumn();
            ImGui::Text("%0.03f", obj.shape.planes[i].v0.x);
            ImGui::SameLine();
            ImGui::Text("%0.03f", obj.shape.planes[i].v0.y);
        }

        ImGui::EndTable();
    }
    ImGui::PopID();
}

template <>
void
draw_component<Segment>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<Segment>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("physics::segment", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 100.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Body Entity:");
            ImGui::TableNextColumn();
            ImGui::InputScalar("##parent", ImGuiDataType_U32, &obj.parent,
                nullptr, nullptr, "%d");
        }

        {
            unsigned type = cpShapeGetCollisionType(obj);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Collision Type:");
            ImGui::TableNextColumn();
            if (ImGui::BeginCombo(
                    "##collision_type", collision_type_name[type])) {
                for (size_t i = 0; i < collision_type_name.size(); i++) {
                    const bool selected = (type == i);
                    if (ImGui::Selectable(collision_type_name[i], selected)) {
                        cpShapeSetCollisionType(obj, i);
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Sensor:");
            ImGui::TableNextColumn();
            ImGui::Checkbox("##sensor", (bool*)&obj.shape.shape.sensor);
        }
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Radius:");
            ImGui::TableNextColumn();
            ImGui::InputScalar("##radius", ImGuiDataType_Float, &obj.shape.r);
        }

        // grab both points early as we need both to edit either one
        cpVect a = cpSegmentShapeGetA(obj);
        cpVect b = cpSegmentShapeGetB(obj);

        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Body A:");
            ImGui::TableNextColumn();
            cpVect orig = a;
            ImGui::DragScalarN("##pos", ImGuiDataType_Float, &a.x, 2, 1.0f,
                nullptr, nullptr, "%0.3f");
            if (a.x != orig.x || a.y != orig.y) {
                cpSegmentShapeSetEndpoints(obj, a, b);
            }
        }
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Body B:");
            ImGui::TableNextColumn();
            cpVect orig = b;
            ImGui::DragScalarN("##pos", ImGuiDataType_Float, &b.x, 2, 1.0f,
                nullptr, nullptr, "%0.3f");
            if (b.x != orig.x || b.y != orig.y) {
                cpSegmentShapeSetEndpoints(obj, a, b);
            }
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Space A:");
        ImGui::TableNextColumn();
        cpVect ta = obj.ta();
        ImGui::Text("(%0.3f, %0.3f)", ta.x, ta.y);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Space B:");
        ImGui::TableNextColumn();
        cpVect tb = obj.tb();
        ImGui::Text("(%0.3f, %0.3f)", tb.x, tb.y);

        ImGui::EndTable();
    }
    ImGui::PopID();
}

template <>
void
draw_component<Destination>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<Destination>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("physics::destination", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Position");
        ImGui::TableNextColumn();
        ImGui::DragScalarN("##pos", ImGuiDataType_Float, &obj.pos.x, 2, 1.0f,
            nullptr, nullptr, "%0.3f");
        ImGui::EndTable();
    }
    ImGui::PopID();
}

template <>
void
draw_component<Accelerate>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<Accelerate>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("physics::accelerate", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Force");
        ImGui::TableNextColumn();
        ImGui::DragScalarN("##force", ImGuiDataType_Float, &obj.force.x, 2,
            1.0f, nullptr, nullptr, "%0.3f");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Cap");
        ImGui::TableNextColumn();
        ImGui::DragScalarN("##cap", ImGuiDataType_Float, &obj.cap, 1, 1.0f,
            nullptr, nullptr, "%0.3f");

        ImGui::EndTable();
    }
    ImGui::PopID();
}

template <>
void
draw_component<Movable>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<Movable>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("physics::movable", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Acceleration");
        ImGui::TableNextColumn();
        ImGui::DragScalarN("##accel", ImGuiDataType_Float, &obj.accel, 1,
            1.0f, nullptr, nullptr, "%0.3f");

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Velocity Max");
        ImGui::TableNextColumn();
        ImGui::DragScalarN("##max_vel", ImGuiDataType_Float,
            &obj.velocity_max, 1, 1.0f, nullptr, nullptr, "%0.3f");

        ImGui::EndTable();
    }
    ImGui::PopID();
}

template <>
void
draw_component<Collision>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<Collision>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("physics::collision", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("Normal");
        ImGui::TableNextColumn();
        ImGui::DragScalarN("##normal", ImGuiDataType_Float, &obj.normal, 2,
            1.0f, nullptr, nullptr, "%0.3f");

        ImGui::EndTable();
    }
    ImGui::PopID();
}

} // namespace entity_editor
