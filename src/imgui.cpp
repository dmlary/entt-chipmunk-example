#include <chrono>
#include <entt/entt.hpp>
#include <imgui.h>
#include <sokol_app.h>

#include "asset_loader.hpp"
#include "components.hpp"
#include "entity_editor.hpp"
#include "imgui.hpp"
#include "log.hpp"
#include "render.hpp"
#include "system.hpp"
#include "tags.hpp"

template <>
void
entity_editor::draw_component<imgui::Draw>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<imgui::Draw>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("imgui::draw", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("draw");
        ImGui::TableNextColumn();
        ImGui::Text("%p", &obj.draw);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("expire_frame");
        ImGui::TableNextColumn();
        ImGui::DragScalarN(
            "##expire_frame", ImGuiDataType_U64, &obj.expire_frame, 1);

        ImGui::EndTable();
    }
    ImGui::PopID();
}

namespace imgui {

plugin::plugin(entt::registry&)
{
    log_debug("load imgui plugin");
    const simgui_desc_t desc{};
    simgui_setup(&desc);
    const sg_imgui_desc_t sg_imgui_desc{};
    sg_imgui_init(&m_sg_imgui, &sg_imgui_desc);

    m_editor_entity = entt::null;
}

plugin::~plugin()
{
    log_debug("destroy imgui plugin");
    sg_imgui_discard(&m_sg_imgui);
    simgui_shutdown();
}

void
plugin::init(entt::registry& ecs)
{
    if (ecs.ctx().contains<entity_editor::plugin>()) {
        auto& editor = ecs.ctx().get<entity_editor::plugin>();
        editor.add<Draw>("imgui::Draw");
    }

    entt::entity entity = ecs.create();
    ecs.emplace<System>(entity,
        System::Config<Draw>{
            .name       = "imgui::draw",
            .always_run = true,
            .stage      = System::Stage::imgui_draw,
            .handler =
                [](auto& ecs, auto& view, float delta) {
                    simgui_new_frame({ .width = sapp_width(),
                        .height               = sapp_height(),
                        .delta_time           = delta,
                        .dpi_scale            = sapp_dpi_scale() });

                    // grab the current frame number for use with
                    // Draw.expire_frame
                    uint64_t frame = sapp_frame_count();

                    // call all the draw functions
                    for (auto&& [entity, draw] : view.each()) {
                        if (draw.expire_frame != 0
                            && draw.expire_frame <= frame) {
                            ecs.destroy(entity);
                            continue;
                        }
                        draw.draw(ecs, entity, draw);
                    }

                    // now render it all in one pass
                    sg_pass_action pass = {
                        .colors[0].action = SG_ACTION_DONTCARE,
                    };
                    sg_begin_default_pass(&pass, sapp_width(), sapp_height());
                    simgui_render();
                    sg_end_pass();
                },
        });
    ecs.emplace<HumanDescription>(
        entity, "system: ImGui", "render all imgui::Draw components");

    // sokol debug stuff
    entity = ecs.create();
    ecs.emplace<Draw>(entity, [this](auto&, auto, auto&) {
        sg_imgui_draw(&m_sg_imgui);
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("sokol-gfx")) {
                ImGui::MenuItem("Capabilities", 0, &m_sg_imgui.caps.open);
                ImGui::MenuItem("Buffers", 0, &m_sg_imgui.buffers.open);
                ImGui::MenuItem("Images", 0, &m_sg_imgui.images.open);
                ImGui::MenuItem("Shaders", 0, &m_sg_imgui.shaders.open);
                ImGui::MenuItem("Pipelines", 0, &m_sg_imgui.pipelines.open);
                ImGui::MenuItem("Passes", 0, &m_sg_imgui.passes.open);
                ImGui::MenuItem("Calls", 0, &m_sg_imgui.capture.open);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    });
    ecs.emplace<HumanDescription>(entity, "imgui: sokol debugging",
        "Draw sokol debugging menu & windows");

    // tools list
    auto& editor = ecs.ctx().get<entity_editor::plugin>();
    entity       = ecs.create();
    ecs.emplace<Draw>(entity, [this, editor](auto& ecs, auto, auto&) {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("tools")) {
                ImGui::MenuItem("systems", 0, &m_systems);
                ImGui::MenuItem("entities", 0, &m_editor);
                ImGui::MenuItem("ImGui Demo", 0, &m_demo);
                if (ImGui::MenuItem("Reset Scene")) {
                    auto& loader = ecs.ctx().template get<asset_loader>();
                    loader.clear_scene(ecs);
                    loader.load_scene(ecs);
                    auto& render = ecs.ctx().template get<render::plugin>();
                    render.reset_camera(ecs);
                }
                if (ImGui::MenuItem("Clear Scene")) {
                    auto& loader = ecs.ctx().template get<asset_loader>();
                    loader.clear_scene(ecs);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        if (m_demo) {
            ImGui::ShowDemoWindow(&m_demo);
        }
        if (m_systems) {
            show_systems(ecs);
        }
        if (m_editor) {
            editor.draw_editor(ecs, m_editor_entity, m_editor);
        }
    });
    ecs.emplace<HumanDescription>(
        entity, "imgui: game debugging", "Draw game debugging menu & windows");
}

void
plugin::show_systems(entt::registry& ecs)
{
    ImGui::Begin("Systems", &m_systems);

    system_step_state &state = ecs.ctx().get<system_step_state>();
    ImGui::Text("Debug Mode");
    ImGui::SameLine();
    ImGui::Checkbox("##single-step", &state.enabled);

    if (state.enabled) {
        ImGui::SameLine();
        if (ImGui::Button("Step")) {
            state.step = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Next Frame")) {
            state.frame = true;
        }
    } else {
        state.next_system = entt::null;
    }

    if (ImGui::BeginTable(
            "systems", 6, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("E", ImGuiTableColumnFlags_WidthFixed, 20.0f);
        ImGui::TableSetupColumn("A", ImGuiTableColumnFlags_WidthFixed, 20.0f);
        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn(
            "time (ms)", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn(
            "max (ms)", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn(
            "entities", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableHeadersRow();

        for (auto&& [e, system] : ecs.view<System>().each()) {
            ImGui::PushID(&system);
            ImGui::TableNextRow();
            if (e == state.next_system) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, 0xFFFFB8BF);
            }
            ImGui::TableNextColumn();
            ImGui::Checkbox("##enabled", &system.enabled);
            ImGui::TableNextColumn();
            ImGui::Checkbox("##always_run", &system.always_run);
            ImGui::TableNextColumn();
            ImGui::Text("%s", system.name);
            ImGui::TableNextColumn();
            ImGui::Text("%0.03f", system.perf.last.count() / 1000000.0);
            ImGui::TableNextColumn();
            ImGui::Text("%0.03f", system.perf.max.count() / 1000000.0);
            ImGui::TableNextColumn();
            ImGui::Text("%zu", system.perf.entities);
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

void
notify(entt::registry& ecs, const char* msg, unsigned frames)
{
    auto e     = ecs.create();
    auto& draw = ecs.emplace<Draw>(e, [msg](auto&, auto, auto&) {
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (ImGui::Begin("notification", nullptr, window_flags)) {
            ImGui::Text("%s", msg);
        }
        ImGui::End();
    });

    draw.expire_frame = sapp_frame_count() + frames;
}

} // namespace imgui
