#version 430 core

// Structures.
struct Transform
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

struct DLight
{
	vec3 direction;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

// Uniforms.
uniform Transform u_transform;
uniform DLight u_light_sun;

// Attributes.
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;

// Outputs.
out vec2 a_texture;
out vec3 a_frag_pos;
out vec3 a_frag_normal;
out float diffuseIntensity;

void main()
{	
	// Calculate diffuse light intensity.
	diffuseIntensity = max(0.0, dot(a_normal, u_light_sun.direction));

	// Calculate vertex world position for specular will be interpolated to be correct fragment position.
	a_frag_pos = (u_transform.view * u_transform.model * vec4(a_position, 1.0f)).xyz;
	a_frag_normal = a_normal;
	
	// Set texture uv and vertex position.
	a_texture = a_uv;
	gl_Position = u_transform.projection * u_transform.view * u_transform.model * vec4(a_position, 1.0f);
}