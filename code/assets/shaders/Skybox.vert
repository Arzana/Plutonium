#version 430 core

// Uniforms.
uniform mat4 u_view;
uniform mat4 u_projection;

// Inputs.
in vec3 a_position;

// Outputs.
out vec3 a_texture;

void main()
{
	a_texture = a_position;
	gl_Position = (u_projection * u_view * vec4(a_position, 1.0f)).xyww;
}