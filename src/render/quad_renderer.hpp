#pragma once

#include <map>
#include <glm/glm.hpp>
#include <sokol_gfx.h>
#include "rgba.hpp"
#include "buffer.hpp"

namespace render {

class QuadRenderer
{
public:
    // maximum number of textures the shader supports
    const size_t ImagesMax = 4;

    using rgba = glm::vec<4, uint8_t>;

    struct Vertex
    {
        glm::vec<2, uint16_t> pos;
        float texture;
        glm::vec2 texture_pos;
        rgba color;
    };

    struct BufferPair {
        Buffer<Vertex> v;
        IndexBuffer i;
    };

    QuadRenderer(size_t v_max, size_t i_max);
    ~QuadRenderer();
    void clear();
    void render(glm::mat4&);
    void draw_rect(
        glm::vec2 pos,
        glm::vec2 size,
        sg_image texture,
        glm::mat3 texture_transform);

private:

    sg_pipeline_desc m_pipeline_desc;
    sg_pipeline m_pipeline;
    sg_shader m_shader;
    sg_buffer_desc m_vb_desc, m_ib_desc;
    sg_buffer m_vb, m_ib;
    sg_bindings m_bindings{};
    sg_image m_white_img;
    std::map<uint32_t, size_t> m_image_map{};
    BufferPair m_buf;
};

} // namespace render
