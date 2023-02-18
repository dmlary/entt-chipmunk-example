#include "entity_editor.hpp"
#include "components.hpp"
#include "imgui.hpp"
#include "log.hpp"
#include "system.hpp"
#include "tags.hpp"
#include <entt/entt.hpp>
#include <imgui.h>

namespace entity_editor {

void
plugin::draw_entity_editor(entt::registry& ecs, entt::entity entity) const
{
    if (!ecs.valid(entity)) {
        return;
    }
    ImGui::BeginChild("editor");
    for (auto& [type_id, cfg] : m_components) {
        const auto* storage = ecs.storage(type_id);
        if (storage && storage->contains(entity)) {
            if (ImGui::CollapsingHeader(cfg.name)) {
                cfg.draw(ecs, entity);
            }
        }
    }
    ImGui::EndChild();
}

void
plugin::draw_editor(entt::registry& ecs,
    entt::entity& entity,
    bool& open) const
{
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("entity editor", &open)) {
        ImGui::PushID(&entity);
        if (ImGui::BeginChild("list", { 200, 0 }, true)) {
            auto desc_view = ecs.view<HumanDescription>();
            ecs.each([this, &ecs, &entity, &desc_view](auto e) {
                char buf[128];
                if (desc_view.contains(e)) {
                    auto& desc = desc_view.get<HumanDescription>(e);
                    snprintf(buf, sizeof(buf), "%d %s", e, desc.name.c_str());
                } else {
                    snprintf(buf, sizeof(buf), "%d", e);
                }
                if (ImGui::Selectable(buf, e == entity,
                        ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        create_editor(ecs, e);
                    } else {
                        entity = e;
                    }
                }
            });
            ImGui::EndChild();
        }
        ImGui::SameLine();

        draw_entity_editor(ecs, entity);

        ImGui::PopID();
    }
    ImGui::End();
}

void
plugin::create_editor(entt::registry& ecs, entt::entity entity) const
{
    std::string name    = fmt::format("entity_editor: {}", (uint32_t)entity);
    entt::entity drawer = ecs.create();
    ecs.emplace<HumanDescription>(
        drawer, name, "independent entity editor window");
    ecs.emplace<imgui::Draw>(
        drawer, [this, name, entity](auto& ecs, auto draw_entity, auto&) {
            bool show{ true };
            ImGui::SetNextWindowSize(ImVec2(300, 440), ImGuiCond_FirstUseEver);
            if (ImGui::Begin(name.c_str(), &show)) {
                draw_entity_editor(ecs, entity);
            }
            ImGui::End();
            if (!show) {
                ecs.template emplace<tags::Destroy>(draw_entity);
            }
        });
}

/// editor for HumanDescription
template <>
void
draw_component<HumanDescription>(entt::registry& ecs, entt::entity e)
{
    auto& obj = ecs.get<HumanDescription>(e);
    ImGui::PushID(&obj);
    if (ImGui::BeginTable("HumanDescription", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("name");
        ImGui::TableNextColumn();
        ImGui::Text("%s", obj.name.c_str());

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("desc");
        ImGui::TableNextColumn();
        ImGui::Text("%s", obj.desc.c_str());

        ImGui::EndTable();
    }
    ImGui::PopID();
}

/// editor for System
template <>
void
draw_component<System>(entt::registry& ecs, entt::entity e)
{
    auto& sys = ecs.get<System>(e);
    ImGui::PushID(&sys);
    if (ImGui::BeginTable("system", 2, 0)) {
        ImGui::TableSetupColumn(
            "field", ImGuiTableColumnFlags_WidthFixed, 75.0);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("name");
        ImGui::TableNextColumn();
        ImGui::Text("%s", sys.name);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("enabled");
        ImGui::TableNextColumn();
        ImGui::Checkbox("##enabled", &sys.enabled);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("stage");
        ImGui::TableNextColumn();
        ImGui::DragScalar("##stage", ImGuiDataType_U32, &sys.stage, 100);

        // cannot display address of run; lambda addr not accessile via
        // fmt::function::value(); always returns null.

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("entities");
        ImGui::TableNextColumn();
        ImGui::Text("%zu", sys.perf.entities);

        ImGui::EndTable();
    }
    ImGui::PopID();
}

}
