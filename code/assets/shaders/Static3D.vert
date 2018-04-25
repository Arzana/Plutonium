#version 430 core

// Structures.
struct Transform
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

struct LightInfo
{
	vec3 reflectedDirection;
	float diffuseIntensity;
};

// Uniforms.
uniform Transform u_transform;
uniform vec3 u_light_direction;

// Attributes.
in vec3 a_position;
in vec3 a_normal;
in vec2 a_uv;

// Outputs.
out vec2 a_texture;
out vec3 a_vrtx_pos;
out LightInfo a_light;

void main()
{	
	// Calculate diffuse light intensity.
	a_light.diffuseIntensity = max(0.0, dot(a_normal, u_light_direction));

	// Calculate reflected light direction and vertex world position for specular.
	a_light.reflectedDirection = reflect(-u_light_direction, a_normal);
	a_vrtx_pos = (u_transform.view * u_transform.model * vec4(a_position, 1.0f)).xyz;
	
	// Set texture uv and vertex position.
	a_texture = a_uv;
	gl_Position = u_transform.projection * u_transform.view * u_transform.model * vec4(a_position, 1.0f);
}