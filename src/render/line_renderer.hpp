#pragma once

#include <map>
#include <glm/glm.hpp>
#include <sokol_gfx.h>
#include "rgba.hpp"
#include "buffer.hpp"

namespace render {

class LineRenderer
{
public:

    struct Vertex
    {
        glm::vec2 pos;
        rgba color;
    };

    struct BufferPair {
        Buffer<Vertex> v;
        IndexBuffer i;
    };


    LineRenderer(
            size_t v_max,
            size_t i_max);
    ~LineRenderer();
    void clear();
    void render(const glm::mat4&);
    void draw_rect(glm::vec2 pos, glm::vec2 size, rgba color);
    void draw_seg(glm::vec2 a, glm::vec2 b, rgba color);

private:
    sg_pipeline_desc m_pipeline_desc;
    sg_pipeline m_pipeline;
    sg_shader m_shader;
    sg_buffer_desc m_vb_desc, m_ib_desc;
    sg_buffer m_vb, m_ib;
    BufferPair m_buf;
};

} // namespace render
