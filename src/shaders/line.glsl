@header #include <glm/glm.hpp>
@ctype mat4 glm::mat4

@vs vs
@glsl_options flip_vert_y
layout(location=0) in vec4 v_pos;
layout(location=1) in vec4 v_color;

out vec4 f_color;

uniform vs_params {
    mat4 u_mvp;
};

void main() {
    gl_Position = u_mvp * v_pos;
    f_color = v_color;
}
@end

@fs fs
in vec4 f_color;

out vec4 frag_color;

layout(location=0) uniform sampler2D u_texture;

void main() {
    frag_color = f_color;
}
@end

@program line vs fs
