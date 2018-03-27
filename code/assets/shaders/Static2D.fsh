#version 430 core

// Uniforms.
uniform sampler2D u_texture;
uniform vec4 u_color;

// Inputs.
in vec2 a_texture;

// Outputs.
out vec4 fragColor;

void main()
{
	// Set output.
	fragColor = texture(u_texture, a_texture) * u_color;
}