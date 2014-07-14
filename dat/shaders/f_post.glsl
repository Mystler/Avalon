#version 150

in vec2 p_uv;

out vec4 o_color;

uniform sampler2D u_tex;

void main() {
    o_color = texture(u_tex, p_uv);
}
