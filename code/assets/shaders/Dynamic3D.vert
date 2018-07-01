#version 430 core

// Uniforms.
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_light_direction;
uniform float u_ambient;
uniform float u_time;

// Attributes.
in vec3 a_position_1;
in vec3 a_position_2;
in vec3 a_normal_1;
in vec3 a_normal_2;
in vec2 a_uv;

// Outputs.
out vec2 a_texture;
out float a_intensity;

void main()
{	
	// Calculate light intensity.
	vec3 normal = (u_model * vec4(mix(a_normal_1, a_normal_2, u_time), 0.0f)).xyz;
	a_intensity = u_ambient + max(0.0, dot(normal, u_light_direction));
	
	// Set texture uv and vertex position.
	a_texture = a_uv;
	gl_Position = u_projection * u_view * u_model * vec4(mix(a_position_1, a_position_2, u_time), 1.0);
}