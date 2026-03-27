#version 460 core

layout(location=0) in vec3 in_pos;
layout(location=1) in vec2 in_uv;

out vec2 out_uv;

uniform float u_time;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
     gl_Position = u_projection * u_view * u_model * vec4(in_pos, 1.0f);
     out_uv = in_uv;
}