#include <map>
#include <entt/entt.hpp>

#pragma once

namespace entity_editor {

/// common function to draw a component
template <typename T>
void
draw_component(entt::registry&, entt::entity) {};

/// configuration for how to handle a component in editor
struct Config {
    const char *name;
    std::function<void(entt::registry&, entt::entity)> draw;
};

class plugin {
public:
    plugin(entt::registry&) {};

    /// register a component with the entity editor
    template <typename T>
    void add(const char *name) {
        Config c = {
            .name = name,
            .draw = &draw_component<T>,
        };
        auto type = entt::type_hash<T>::value();
        m_components.insert_or_assign(type, c);
    }

    /// draw an editor for all entities
    void draw_editor(entt::registry&, entt::entity&, bool&) const;

    /// draw a single entity editor
    void draw_entity_editor(entt::registry& ecs, entt::entity entity) const;

    /// create a new entity editing window in the registry
    void create_editor(entt::registry&, entt::entity) const;

private:
    std::map<entt::id_type, Config> m_components;
};

} // namespace entity_editor
