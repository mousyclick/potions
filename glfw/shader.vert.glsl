#version 460 core

layout(location=0) in vec3 pos;

uniform float u_time;

void main() {
     gl_Position = vec4(sin(pos.x * u_time), pos.y + sin(u_time) / 2, -1, 1.0);
}