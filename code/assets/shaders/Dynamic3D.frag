#version 430 core

// Uniforms.
uniform sampler2D u_texture;

// Inputs.
in float a_intensity;
in vec2 a_texture;

// Outputs.
out vec4 fragColor;

void main()
{
	// Set output.
	fragColor = texture(u_texture, a_texture) * vec4(a_intensity, a_intensity, a_intensity, 1.0);
}