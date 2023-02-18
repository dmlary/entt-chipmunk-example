#pragma once

#include <entt/entt.hpp>

namespace physics {

class plugin {
public:
    plugin(entt::registry&);
    ~plugin() {};

    void init(entt::registry&);
    void update(entt::registry&, float delta);
    void cleanup(entt::registry&) {};

private:
    entt::entity m_system_step{};
};

} // namespace render
