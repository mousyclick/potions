#version 460 core

in vec4 out_color;
in vec2 out_uv;

out vec4 frag_color;

uniform sampler2D u_container_texture;
uniform sampler2D u_face_texture;

uniform float u_time;

void main() {
  frag_color = mix(texture(u_container_texture, out_uv), texture(u_face_texture, out_uv * vec2(sin(u_time), 1.0f)), 0.5);
}