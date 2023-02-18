#include <entt/entt.hpp>
#include <imgui.h>

#include "components.hpp"
#include "log.hpp"
#include "input.hpp"
#include "system.hpp"
#include "physics.hpp"
#include "imgui.hpp"

namespace input {

using namespace entt::literals;

/// handle any pending input
static void
handle_input(entt::registry& ecs, float)
{
    const auto move = physics::move_entity_cardinal;

    if (ImGui::IsKeyPressed(ImGuiKey_S)) {
        system_step_state &state = ecs.ctx().get<system_step_state>();
        state.enabled = !state.enabled;
        state.step = false;

        imgui::notify(ecs, state.enabled ?
                "enabled system step-mode" :
                "disabled system step-mode");
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
        system_step_state &state = ecs.ctx().get<system_step_state>();
        if (state.enabled) {
            state.frame = true;
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        auto player = ecs.ctx().get<entt::entity>("player"_hs);
        move(ecs, player, physics::CD_South);
    } else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        auto player = ecs.ctx().get<entt::entity>("player"_hs);
        move(ecs, player, physics::CD_North);
    } else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        auto player = ecs.ctx().get<entt::entity>("player"_hs);
        move(ecs, player, physics::CD_West);
    } else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        auto player = ecs.ctx().get<entt::entity>("player"_hs);
        move(ecs, player, physics::CD_East);
    }
}

void
plugin::init(entt::registry& ecs)
{
    auto entity = ecs.create();
    ecs.emplace<System>(entity, System::Config<>{
            .name = "input",
            .enabled = true,
            .always_run = true,
            .stage = System::Stage::input,
            .handler = handle_input,
        });
    ecs.emplace<HumanDescription>(entity,
            "System: Input",
            "process player input");
}

} // namespace input
