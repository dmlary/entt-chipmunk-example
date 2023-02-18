#pragma once

#include <entt/fwd.hpp>
#include <imgui.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <util/sokol_gfx_imgui.h>
#include <util/sokol_imgui.h>

namespace imgui {

struct Draw {
    std::function<void(entt::registry&, entt::entity, Draw&)> draw;

    /// frame number (`sapp_frame_count()`) at which to destroy the entity
    /// holding this Draw component.  This field is defined directly on Draw to
    /// allow UI elements to continue functioning while systems are in
    /// step-mode.
    uint64_t expire_frame{0};
};

class plugin {
public:
    plugin(entt::registry&);
    ~plugin();

    void init(entt::registry&);
    void cleanup(entt::registry&){};

private:
    void show_systems(entt::registry&);
    void draw_overlay(entt::registry&);

    bool m_demo{ false }; // open imgui demo
    bool m_systems{ false }; // open systems monitor
    bool m_editor{ false }; // open entity editor
    entt::entity m_editor_entity;
    sg_imgui_t m_sg_imgui{};
};

/// show a notification message on the screen for a set number of frames
void notify(entt::registry&, const char *msg, unsigned frames=150);

}
