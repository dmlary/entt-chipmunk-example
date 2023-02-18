#include "quad_renderer.hpp"
#include "shaders/quad.glsl.h"
#include "../log.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace render {

QuadRenderer::QuadRenderer(size_t v_max, size_t i_max)
{
    assert(v_max < (2<<16) && "v_max exceeds uint16 index buffer");

    /* create a pipeline object (default render state is fine) */
    m_pipeline_desc = {
        .label = "QuadRenderer",
        .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .attrs = {
                [ATTR_vs_v_pos].format =         SG_VERTEXFORMAT_SHORT2,
                [ATTR_vs_v_texture].format =     SG_VERTEXFORMAT_FLOAT,
                [ATTR_vs_v_texture_pos].format = SG_VERTEXFORMAT_FLOAT2,
                [ATTR_vs_v_color].format =       SG_VERTEXFORMAT_UBYTE4N,
            }
        },
        .colors[0].blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        },
    };

    m_shader = sg_make_shader(quad_shader_desc(sg_query_backend()));
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

    // create the white pixel image
    sg_image_desc m_desc = {
        .label = "QuadRenderer::m_white_img",
        .width = 1,
        .height = 1,
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .data.subimage[0][0] = {
            .size = 4,
            .ptr = "\xff\xff\xff\xff",
        },
    };
    m_white_img = sg_make_image(&m_desc);

    clear();
}

QuadRenderer::~QuadRenderer()
{
    log_debug("destroy shader {}, pipeline {}, ib {}, vb {}",
            m_shader.id, m_pipeline.id, m_ib.id, m_vb.id);
    sg_destroy_buffer(m_ib);
    sg_destroy_buffer(m_vb);
    sg_destroy_pipeline(m_pipeline);
    sg_destroy_shader(m_shader);
}

void
QuadRenderer::clear()
{
    m_buf.v.clear();
    m_buf.i.clear();
    m_image_map.clear();
}

void
QuadRenderer::render(glm::mat4& proj)
{
    if (m_buf.i.size() == 0) {
        return;
    }

    sg_apply_pipeline(m_pipeline);

    // set the uniforms
    vs_params_t params{ .u_mvp = proj };
    sg_range p = SG_RANGE(params);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &p);

    m_bindings = {
        .vertex_buffers[0] = m_vb,
        .vertex_buffer_offsets[0] =
            sg_append_buffer(m_vb, m_buf.v.sg_range()),
        .index_buffer = m_ib,
        .index_buffer_offset =
            sg_append_buffer(m_ib, m_buf.i.sg_range()),
    };

    for (size_t i = 0; i < ImagesMax; i++) {
        m_bindings.fs_images[i] = m_white_img;
    }

    // update our textures
    for (const auto &p : m_image_map) {
        m_bindings.fs_images[p.second] = {p.first};
    }

    // bind & draw
    sg_apply_bindings(&m_bindings);
    sg_draw(0, m_buf.i.elements(), 1);
}

void
QuadRenderer::draw_rect(
        glm::vec2 pos,
        glm::vec2 size,
        sg_image texture,
        glm::mat3 t_transform)
{
    static const glm::vec3 quad[] = {
        {  0.0f,  0.0f, 1.0f },
        {  1.0f,  0.0f, 1.0f },
        {  1.0f,  1.0f, 1.0f },
        {  0.0f,  1.0f, 1.0f },
    };
    assert(texture.id != SG_INVALID_ID);

    int t_idx = -1;
    if (const auto &p = m_image_map.find(texture.id); p != m_image_map.end()) {
        t_idx = p->second;
    } else {
        t_idx = m_image_map[texture.id] = m_image_map.size();
        // log_debug("added texture {} -> index {}", texture.id, t_idx);
    }
    assert(m_image_map.size() <= ImagesMax && "too many images");

    glm::mat3 transform{
        { static_cast<float>(size.x), 0.0f, 0.0f },
        { 0.0f, static_cast<float>(size.y), 0.0f },
        { static_cast<float>(pos.x), static_cast<float>(pos.y), 1.0f },
    };

    uint16_t base = m_buf.v.elements();
    for (int i = 0; i < 4; i++) {
        //log_debug("quad[{}] {} -> {}", i, quad[i], transform * quad[i]);
        m_buf.v.push_back({
                transform * quad[i],
                static_cast<float>(t_idx),
                t_transform * quad[i],
                {255, 255, 255, 255},
            });
    }
    m_buf.i.append({
            (uint16_t)base, (uint16_t)(base + 1), (uint16_t)(base + 2),
            (uint16_t)base, (uint16_t)(base + 3), (uint16_t)(base + 2),
        });
}

} // namespace render
