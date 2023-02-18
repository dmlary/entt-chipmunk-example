#include "line_renderer.hpp"
#include "shaders/line.glsl.h"
#include "../log.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace render {

LineRenderer::LineRenderer(size_t v_max, size_t i_max)
{
    assert(v_max < (2<<16) && "v_max exceeds uint16 index buffer");

    /* create a pipeline object (default render state is fine) */
    m_pipeline_desc = {
        .label = "LineRenderer",
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .attrs = {
                [0] = { .offset=0, .format=SG_VERTEXFORMAT_FLOAT2 },
                [1] = { .offset=8, .format=SG_VERTEXFORMAT_UBYTE4N},
            }
        },
        .colors[0].blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        },
    };

    m_shader = sg_make_shader(line_shader_desc(sg_query_backend()));
    m_pipeline_desc.shader = m_shader;
    m_pipeline = sg_make_pipeline(&m_pipeline_desc);

    m_vb_desc = {
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .size = v_max * sizeof(Vertex),
    };
    m_vb = sg_make_buffer(&m_vb_desc);

    m_ib_desc = {
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = SG_USAGE_DYNAMIC,
        .size = i_max * sizeof(uint16_t),
    };
    m_ib = sg_make_buffer(&m_ib_desc);
}

LineRenderer::~LineRenderer()
{
    log_debug("destroy shader {}, pipeline {}, ib {}, vb {}",
            m_shader.id, m_pipeline.id, m_ib.id, m_vb.id);
    sg_destroy_buffer(m_ib);
    sg_destroy_buffer(m_vb);
    sg_destroy_pipeline(m_pipeline);
    sg_destroy_shader(m_shader);
}

void
LineRenderer::clear()
{
    m_buf.v.clear();
    m_buf.i.clear();
}

void
LineRenderer::render(const glm::mat4& proj)
{
    if (m_buf.i.size() == 0) {
        return;
    }
    sg_apply_pipeline(m_pipeline);

    sg_range p = SG_RANGE(proj);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &p);    // XXX magic number

    sg_bindings bind{
        .vertex_buffers[0] = m_vb,
        .vertex_buffer_offsets[0] = sg_append_buffer(m_vb, m_buf.v.sg_range()),
        .index_buffer = m_ib,
        .index_buffer_offset = sg_append_buffer(m_ib, m_buf.i.sg_range()),
    };
    sg_apply_bindings(&bind);
    sg_draw(0, m_buf.i.elements(), 1);
}

void
LineRenderer::draw_rect(glm::vec2 pos, glm::vec2 size, rgba color)
{
    static const glm::vec3 quad[] = {
        {  0.0f,  0.0f, 1.0f },
        {  1.0f,  0.0f, 1.0f },
        {  1.0f,  1.0f, 1.0f },
        {  0.0f,  1.0f, 1.0f },
    };

    glm::mat3 transform{
        { static_cast<float>(size.x), 0.0f, 0.0f },
        { 0.0f, static_cast<float>(size.y), 0.0f },
        { static_cast<float>(pos.x), static_cast<float>(pos.y), 1.0f },
    };

    uint16_t base = m_buf.v.elements();
    for (int i = 0; i < 4; i++) {
        m_buf.v.push_back({ transform * quad[i], color});
    }
    m_buf.i.append({
            (uint16_t)(base + 0), (uint16_t)(base + 1),
            (uint16_t)(base + 1), (uint16_t)(base + 2),
            (uint16_t)(base + 2), (uint16_t)(base + 3),
            (uint16_t)(base + 3), (uint16_t)(base + 0),
        });
}

void
LineRenderer::draw_seg(glm::vec2 a, glm::vec2 b, rgba color)
{
    uint16_t base = m_buf.v.elements();
    m_buf.v.append({ { a, color }, { b, color }});
    m_buf.i.append({ (uint16_t)(base + 0), (uint16_t)(base + 1), });
}

} // namespace render
