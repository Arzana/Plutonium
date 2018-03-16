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
in vec2 a_normal_1;
in vec2 a_normal_2;
in vec2 a_uv;

// Outputs.
out vec2 a_texture;
out float a_intensity;

void main()
{
	// Construct normals and move it to model space.
	float z_1 = sqrt(1.0 - (a_normal_1.x * a_normal_1.x) - (a_normal_1.y * a_normal_1.y));
	float z_2 = sqrt(1.0 - (a_normal_2.x * a_normal_2.x) - (a_normal_2.y * a_normal_2.y));
	vec3 normal_1 = normalize((u_model * vec4(a_normal_1.x, a_normal_1.y, z_1, 0.0)).xyz);
	vec3 normal_2 = normalize((u_model * vec4(a_normal_2.x, a_normal_2.y, z_2, 0.0)).xyz);
	
	// Calculate light intensity.
	vec3 normal_3 = mix(normal_1, normal_2, u_time);
	a_intensity = u_ambient + max(0.0, dot(normal_3, u_light_direction));
	
	// Set texture uv and vertex position.
	a_texture = a_uv;
	gl_Position = u_projection * u_view * u_model * vec4(mix(a_position_1, a_position_2, u_time), 1.0);
}