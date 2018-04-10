#version 430 core

// Uniforms.
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_light_direction;
uniform float u_ambient;

// Attributes.
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;

// Outputs.
out vec2 a_texture;
out float a_intensity;

void main()
{	
	// Calculate light intensity.
	a_intensity = u_ambient + max(0.0, dot(a_normal, u_light_direction));
	
	// Set texture uv and vertex position.
	a_texture = a_uv;
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
}