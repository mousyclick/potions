#version 460 core

layout(location=0) in vec3 in_pos;
layout(location=1) in vec3 in_color;
layout(location=2) in vec2 in_uv;

out vec4 out_color;
out vec2 out_uv;

uniform float u_time;

void main() {
     gl_Position = vec4(in_pos, 1.0f);
     out_color = vec4(in_color, 1.0f);
     out_uv = in_uv;
}