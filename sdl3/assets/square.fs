#version 460 core

in vec4 out_color;
in vec2 out_uv;

out vec4 frag_color;

uniform sampler2D u_texture;

uniform float u_time;

void main() {
  frag_color = texture(u_texture, out_uv);
}