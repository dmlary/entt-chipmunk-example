#pragma once

#include <entt/fwd.hpp>
#include <entt/entity/entity.hpp>
#include <chipmunk/chipmunk_types.h>
#include "../image.hpp"
#include "rgba.hpp"

namespace render {

class QuadRenderer;
class LineRenderer;

using vec2 = glm::vec2;

class plugin {
public:
    plugin(entt::registry &);
    ~plugin();

    void init(entt::registry&);
    void cleanup(entt::registry&);

    /// reset camera to start position
    void reset_camera(entt::registry&);

    /// draw basic shapes using 1 pixel lines
    void draw_rect(vec2 pos, vec2 size, rgba color);
    void draw_line(vec2 a, vec2 b, rgba color);

private:
    entt::entity create_camera(entt::registry&);
    void set_camera_collision_handler(entt::registry&);
    void move_camera(entt::registry&, entt::entity, cpVect);

    Image m_blank{};
    std::unique_ptr<QuadRenderer> m_quad_renderer{nullptr};
    std::unique_ptr<LineRenderer> m_line_renderer{nullptr};
    entt::entity m_camera{entt::null};
};

} // namespace render
