#version 430 core

// Uniforms.
uniform sampler2D u_texture_ambient;
uniform sampler2D u_texture_diffuse;
uniform sampler2D u_texture_specular;
uniform sampler2D u_texture_alpha;
uniform vec4 u_frag_filter;
uniform vec4 u_refl_ambient;
uniform vec4 u_refl_diffuse;
uniform vec4 u_refl_specular;
uniform float u_spec_exp;
uniform vec3 u_view_pos;

// Inputs.
in float a_intensity;
in vec3 a_refl_dir;
in vec3 a_vrtx_pos;
in vec2 a_texture;

// Outputs.
out vec4 fragColor;

void main()
{
	// Calculate fragment filter and discard fragment if invisible.
	vec4 modifier = u_frag_filter * texture(u_texture_alpha, a_texture);
	if ((modifier.r < 0.1f && modifier.g < 0.1f && modifier.b < 0.1f) || modifier.a < 0.1f) discard;

	// Calculate fragment ambient color.
	vec4 ambient = texture(u_texture_ambient, a_texture) * u_refl_ambient;

	// Calculate fragment diffuse color.
	vec4 diffuse = texture(u_texture_diffuse, a_texture) * u_refl_diffuse * a_intensity;

	// Calculate fragment specular color.
	float power = pow(max(dot(normalize(u_view_pos - a_vrtx_pos), a_refl_dir), 0.0f), u_spec_exp);
	vec4 specular = texture(u_texture_specular, a_texture) * u_refl_specular * power;
	
	// Calculate fragment's final color.  
	fragColor = (ambient + diffuse + specular) * modifier;
}