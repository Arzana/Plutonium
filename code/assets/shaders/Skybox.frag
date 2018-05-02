#version 430 core

// Uniforms.
uniform samplerCube u_skybox;

// Inputs.
in vec3 a_texture;

// Outputs.
out vec4 FragColor;

void main()
{
	FragColor = texture(u_skybox, a_texture);
}