#version 460 core

in vec4 out_color;

out vec4 frag_color;

uniform float u_time;

void main() {
  frag_color = vec4(out_color.zyx, 1.0);
}