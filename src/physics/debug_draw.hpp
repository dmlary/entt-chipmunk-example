#pragma once

#include <entt/fwd.hpp>

namespace physics {

class debug_draw {
public:
    debug_draw(entt::registry&) {};
    void init(entt::registry&);
    void cleanup(entt::registry&);
    void draw(entt::registry&);

private:
    entt::entity m_system;
    bool m_draw_body{true};
    bool m_draw_dest{true};
    bool m_draw_box{true};
    bool m_draw_segment{true};
};

} // namespace render
