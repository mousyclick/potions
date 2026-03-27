#version 460 core

in vec2 out_uv;

out vec4 frag_color;

uniform sampler2D u_texture;
uniform sampler2D u_texture2;

uniform float u_time;

void main() {
  frag_color = mix(texture(u_texture, out_uv), texture(u_texture2, out_uv), sin(u_time));
}