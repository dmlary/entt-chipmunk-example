#include <chrono>
#include <sokol_gfx.h>
#include <sokol_app.h>
#include <sokol_glue.h>
#include <imgui.h>
#include <util/sokol_imgui.h>
#include <util/sokol_gfx_imgui.h>
#include <entt/entt.hpp>

#include "log.hpp"
#include "physics.hpp"
#include "physics/body.hpp"
#include "physics/collision_type.hpp"
#include "physics/shape.hpp"
#include "physics/debug_draw.hpp"
#include "render.hpp"
#include "imgui.hpp"
#include "tags.hpp"
#include "image.hpp"
#include "system.hpp"
#include "components.hpp"
#include "entity_editor.hpp"
#include "input.hpp"
#include "asset_loader.hpp"

void
init(void *data)
{
    entt::registry &ecs = *static_cast<entt::registry *>(data);
    ecs.ctx().emplace<system_step_state>();

    entt::entity entity = ecs.create();
    ecs.emplace<System>(entity, System::Config<tags::Destroy>{
            .name = "destroyer",
            .stage = System::Stage::cleanup,
            .handler = [](auto &ecs, auto& view, float) {
                    for (auto entity: view) {
                        ecs.destroy(entity);
                    }
                },
        });
    ecs.emplace<HumanDescription>(entity, "system: destroyer",
            "destroys any entity with tags::Destroy");

    log_debug("loading plugins");
    ecs.ctx().emplace<physics::plugin>(ecs);
    ecs.ctx().emplace<physics::debug_draw>(ecs);
    ecs.ctx().emplace<render::plugin>(ecs);
    ecs.ctx().emplace<imgui::plugin>(ecs);
    ecs.ctx().emplace<input::plugin>();
    auto &loader = ecs.ctx().emplace<asset_loader>("resources");

    // add core components to entity editor
    auto &editor = ecs.ctx().emplace<entity_editor::plugin>(ecs);
    editor.add<System>("System");
    editor.add<HumanDescription>("HumanDescription");
    editor.add<tags::Destroy>("tags::Destroy");

    log_debug("initializing plugins");
    ecs.ctx().get<physics::plugin>().init(ecs);
    ecs.ctx().get<physics::debug_draw>().init(ecs);
    ecs.ctx().get<render::plugin>().init(ecs);
    ecs.ctx().get<imgui::plugin>().init(ecs);
    ecs.ctx().get<input::plugin>().init(ecs);

    log_debug("loading map");

    loader.load_scene(ecs);

    // create an obstacle);

    // sort the systems so they're executed in order
    ecs.sort<System>([](const auto &lhs, const auto &rhs) {
            return lhs.stage < rhs.stage;
        });

    log_info("init done");
}

void
frame(void *data)
{
    entt::registry &ecs = *static_cast<entt::registry *>(data);
    float delta = sapp_frame_duration();
    system_step_state &step_state = ecs.ctx().get<system_step_state>();

    if (!step_state.enabled) {
        for (auto &&[e, system] : ecs.view<System>().each()) {
            if (system.enabled) {
                system.run(system, ecs, delta);
            }
        }
        return;
    }

    bool run_all = step_state.frame;
    bool run_one = step_state.step;
    step_state.frame = false;
    step_state.step = false;

    if (run_all) {
        step_state.next_system = entt::null;
    }

    for (auto &&[e, system] : ecs.view<System>().each()) {
        if (!system.enabled) {
            continue;
        }

        if (system.always_run || run_all) {
            goto run;
        }

        if (step_state.next_system == entt::null) {
            step_state.next_system = e;
        }

        if (run_one && step_state.next_system == e) {
            step_state.next_system = entt::null;
            run_one = false;
            goto run;
        } else {
            continue;
        }

run:
        system.run(system, ecs, delta);
    }
}

void
cleanup(void *data)
{
    entt::registry &ecs = *static_cast<entt::registry *>(data);

    ecs.ctx().get<asset_loader>().cleanup();
    ecs.ctx().get<physics::plugin>().cleanup(ecs);
    ecs.ctx().get<physics::debug_draw>().cleanup(ecs);
    ecs.ctx().get<imgui::plugin>().cleanup(ecs);
    ecs.ctx().get<render::plugin>().cleanup(ecs);

    delete &ecs;
}

void
handle_input(const sapp_event *event, void *)
{
    if (simgui_handle_event(event)) {
        return;
    }
}

sapp_desc
sokol_main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    log_init();

    return {
        .width = 1280,
        .height = 720,
        .user_data = new entt::registry,
        .init_userdata_cb = init,
        .frame_userdata_cb = frame,
        .cleanup_userdata_cb = cleanup,
        .event_userdata_cb = handle_input,
        .window_title = "game",
        .ios_keyboard_resizes_canvas = false,
        .icon.sokol_default = true,
    };
}
