#version 460 core

layout(location=0) in vec2 in_pos;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
     gl_Position = u_projection * u_view * u_model * vec4(in_pos.x, 0.0, in_pos.y, 1.0f);
}