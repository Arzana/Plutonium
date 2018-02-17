#version 430 core

uniform mat4 u_wvp;

in vec4 a_position;

out vec2 v_texture;

void main()
{
	gl_Position = u_wvp * vec4(a_position.xy, 0.0, 1.0);
	v_texture = a_position.zw;
}