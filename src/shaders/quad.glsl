@header #include <glm/glm.hpp>
@ctype mat4 glm::mat4

@vs vs
@glsl_options flip_vert_y
layout(location=0) in vec4 v_pos;
layout(location=1) in float v_texture;
layout(location=2) in vec2 v_texture_pos;
layout(location=3) in vec4 v_color;

out vec4 f_color;
out flat int f_texture;
out vec2 f_texture_pos;

uniform vs_params {
    mat4 u_mvp;
};

void main() {
    gl_Position = u_mvp * v_pos;
    f_color = v_color;
    f_texture = int(v_texture);
    f_texture_pos = v_texture_pos;
}
@end

@fs fs
in vec4 f_color;
in flat int f_texture;
in vec2 f_texture_pos;

out vec4 frag_color;

// only 4 texture max for the moment
layout(location=0) uniform sampler2D u_texture0;
layout(location=1) uniform sampler2D u_texture1;
layout(location=2) uniform sampler2D u_texture2;
layout(location=3) uniform sampler2D u_texture3;

void main() {
    frag_color = f_color;

    switch (f_texture) {
        case 0:
            frag_color = texture(u_texture0, f_texture_pos);
            break;
        case 1:
            frag_color = texture(u_texture1, f_texture_pos);
            break;
        case 2: frag_color = texture(u_texture2, f_texture_pos); break;
        case 3: frag_color = texture(u_texture3, f_texture_pos); break;
        default:
            frag_color = vec4(1.0, 0.0, 8.0, 1.0);
            break;
    }

    //frag_color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
}
@end

@program quad vs fs
