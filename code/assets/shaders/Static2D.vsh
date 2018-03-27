#version 430 core

// Uniforms
uniform mat4 u_vp;
uniform mat4 u_model;

// Attributes
in vec4 a_pos_uv;

// Outputs.
out vec2 a_texture;

void main()
{
	gl_Position = u_vp * u_model * vec4(a_pos_uv.xy, 0.0, 0.1);
	a_texture = a_pos_uv.zw;
}