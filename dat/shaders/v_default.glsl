#version 150

in vec3 a_pos;
in vec2 a_uv;

out vec2 p_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

void main() {
    p_uv = a_uv;
    gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.f);
}
