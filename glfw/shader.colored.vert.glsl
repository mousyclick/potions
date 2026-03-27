#version 460 core

layout(location=0) in vec3 in_pos;
layout(location=1) in vec3 in_color;

out vec4 out_color;

uniform float u_time;

void main() {
     gl_Position = vec4(sin(in_pos.x * u_time), in_pos.yz, 1.0);
     out_color = vec4(in_color, 1.0f);
}