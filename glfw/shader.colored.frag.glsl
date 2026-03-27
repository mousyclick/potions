#version 460 core

in vec4 out_color;

out vec4 frag_color;

uniform float u_time;

void main() {
 //    float sint = (sin(u_time) / 2.0) + 0.5;
 //    FragColor = vec4(sint, 0.5, 0.2, 0.5);
 //frag_color = fract(step(0.6f, out_color));
 frag_color = vec4(out_color.zyx, out_color.x);
}