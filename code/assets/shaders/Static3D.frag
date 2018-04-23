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
in vec4 gl_FragCoord;
in float a_intensity;
in vec3 a_refl_dir;
in vec2 a_texture;

// Outputs.
out vec4 fragColor;

void main()
{
	// Calculate fragment ambient color.
	vec4 ambient = texture(u_texture_ambient, a_texture) * u_refl_ambient;

	// Calculate fragment diffuse color.
	vec4 diffuse = texture(u_texture_diffuse, a_texture) * u_refl_diffuse * vec4(a_intensity, a_intensity, a_intensity, 1.0f);

	// Calculate fragment specular color.
	vec3 viewDir = normalize(u_view_pos - gl_FragCoord.xyz);
	float power = pow(max(dot(viewDir, a_refl_dir), 0.0f), u_spec_exp);
	vec4 specular = texture(u_texture_specular, a_texture) * u_refl_specular * vec4(power, power, power, 1.0f);

	// Calculate fragment filter.
	vec4 modifier = u_frag_filter * texture(u_texture_alpha, a_texture);
	
	// Calculate fragment's final color.  
	vec4 clr = (ambient + diffuse + specular) * modifier;
	fragColor = min(clr, vec4(1.0f));
}