#version 430 core

// Structures.
struct Transform
{
	mat4 model;
	mat4 view;
	mat4 projection;
};

struct FragInfo
{
	vec3 position;
	vec2 uv;
	mat3 tbn;
};

// Uniforms.
uniform Transform u_transform;

// Attributes.
in vec3 a_position;
in vec3 a_normal;
in vec3 a_tangent;
in vec2 a_uv;

// Outputs.
out FragInfo a_frag;

void main()
{	
	// Calculate vertex world position for specular will be interpolated to be correct fragment position.
	a_frag.position = (u_transform.view * u_transform.model * vec4(a_position, 1.0f)).xyz;
	
	// Calculate the tangent-bitangent-normal matrix.
	vec3 t = normalize((u_transform.model * vec4(a_tangent, 0.0f)).xyz);
	vec3 n = normalize((u_transform.model * vec4(a_normal, 0.0f)).xyz);
	vec3 b = cross(n, t);
	a_frag.tbn = mat3(t, b, n);
	
	// Set texture uv and vertex position.
	a_frag.uv = a_uv;
	gl_Position = u_transform.projection * u_transform.view * u_transform.model * vec4(a_position, 1.0f);
}