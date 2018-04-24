#version 430 core

// Uniforms.
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_light_direction;

// Attributes.
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;

// Outputs.
out vec2 a_texture;
out float a_intensity;
out vec3 a_refl_dir;
out vec3 a_vrtx_pos;

void main()
{	
	// Calculate diffuse light intensity.
	a_intensity = max(0.0, dot(a_normal, u_light_direction));

	// Calculate reflected light direction and vertex world position for specular.
	a_refl_dir = reflect(-u_light_direction, a_normal);
	a_vrtx_pos = (u_view * u_model * vec4(a_position, 1.0f)).xyz;
	
	// Set texture uv and vertex position.
	a_texture = a_uv;
	gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);
}