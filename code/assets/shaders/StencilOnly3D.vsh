#version 430 core

// Uniforms
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

// Attributes
in vec3 a_position;

void main()
{
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
}