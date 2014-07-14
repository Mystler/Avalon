#version 150

in vec2 p_uv;

out vec4 o_color;

uniform sampler2D u_tex1;
uniform sampler2D u_tex2;

void main() {
   o_color = mix(texture(u_tex1, p_uv), texture(u_tex2, p_uv), .5f);
}
