#version 460 core

in vec3 vs_color;

out vec4 frag_color;

void main() {
  frag_color = vec4(vs_color, 1.0);
}