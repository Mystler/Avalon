#version 150

in vec2 a_pos;
in vec2 a_uv;

out vec2 p_uv;

void main() {
    p_uv = a_uv;
    gl_Position = vec4(a_pos, 0.f, 1.f);
}
