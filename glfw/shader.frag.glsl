#version 460 core

out vec4 FragColor;

uniform float u_time;

void main() {
     float sint = (sin(u_time) / 2.0) + 0.5;
     FragColor = vec4(sint, 0.5, 0.2, 0.5);
}