#version 430 core

// Uniforms.
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_light_direction;

// Attributes.
in vec3 a_position;
in vec2 a_normal;
in vec2 a_uv;

// Outputs.
out vec2 a_texture;
out float a_intensity;

void main()
{
	// Construct normal and move it to model space.
	float z = sqrt(1.0 - (a_normal.x * a_normal.x) - (a_normal.y * a_normal.y));
	vec3 normal = normalize((u_model * vec4(a_normal.x, a_normal.y, z, 0.0)).xyz);
	
	// Calculate light intensity.
	a_intensity = max(0.0, dot(normal, u_light_direction));
	
	// Set texture uv and vertex position.
	a_texture = a_uv;
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
}