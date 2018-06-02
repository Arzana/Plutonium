#version 430 core

// Structures.
struct ObjectMaps
{
	sampler2D ambient;
	sampler2D diffuse;
	sampler2D specular;
	sampler2D alpha;
	sampler2D bump;
};

struct ObjectColors
{
	vec4 lfilter;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float specularExponent;
	float displayGamma;
};

struct DLight
{
	vec3 direction;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct PLight
{
	vec3 position;
	vec3 attenuation;

	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
};

struct FragInfo
{
	vec3 position;
	vec2 uv;
	mat3 tbn;
};

// Uniforms.
uniform ObjectMaps u_textures;
uniform ObjectColors u_colors;
uniform DLight u_light_sun;
uniform PLight u_light_vases[4];
uniform vec3 u_view_pos;

// Inputs.
in FragInfo a_frag;

// Outputs.
out vec4 fragColor;

// Calculates the color change for the object from a specified directional light source.
vec4 CalcDirectionalLight(DLight light, vec3 viewDir, vec3 normal)
{
	float intensity = max(0.0f, dot(normal, light.direction));
	vec3 halfwayDir = normalize(light.direction + viewDir);
	float power = pow(max(dot(normal, halfwayDir), 0.0f), u_colors.specularExponent);

	vec4 ambient = texture(u_textures.ambient, a_frag.uv) * u_colors.ambient * light.ambient;
	vec4 diffuse = texture(u_textures.diffuse, a_frag.uv) * u_colors.diffuse * intensity * light.diffuse;
	vec4 specular = texture(u_textures.specular, a_frag.uv).r * u_colors.specular * power * light.specular;

	return ambient + diffuse + specular;
}

// Calculates the color change for the object from a specified point light source.
vec4 CalcPointLight(PLight light, vec3 viewDir, vec3 normal)
{
	float intensity = max(0.0f, dot(normal, normalize(light.position - a_frag.position)));
	float distance = length(light.position - a_frag.position);
	float attenuation = 1.0f / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
	vec3 halfwayDir = normalize(normalize(light.position - a_frag.position) + viewDir);
	float power = pow(max(dot(normal, halfwayDir), 0.0f), u_colors.specularExponent);

	vec4 ambient = texture(u_textures.ambient, a_frag.uv) * u_colors.ambient * light.ambient * attenuation;
	vec4 diffuse = texture(u_textures.diffuse, a_frag.uv) * u_colors.diffuse * intensity * light.diffuse * attenuation;
	vec4 specular = texture(u_textures.specular, a_frag.uv).r * u_colors.specular * power * light.specular * attenuation;

	return ambient + diffuse + specular;
}

void main()
{
	// Calculate fragment filter and discard fragment if invisible.
	vec4 modifier = u_colors.lfilter * texture(u_textures.alpha, a_frag.uv);
	if ((modifier.r < 0.1f && modifier.g < 0.1f && modifier.b < 0.1f) || modifier.a < 0.1f) discard;

	// Calculate general viewing direction towards the fragment.
	vec3 viewDir = normalize(u_view_pos - a_frag.position);

	// Calculate the fragment's normal.
	vec3 normal = texture(u_textures.bump, a_frag.uv).rgb * 2.0f - 1.0f;
	normal = a_frag.tbn * normalize(normal);

	// Apply lighting.
	vec4 outputColor = CalcDirectionalLight(u_light_sun, viewDir, normal);
	for (int i = 0; i < u_light_vases.length(); i++) outputColor += CalcPointLight(u_light_vases[i], viewDir, normal);
	
	// Calculate fragment's final color and apply gamma correction.  
	vec4 clr = outputColor * modifier;
	fragColor = vec4(pow(clr.rgb, vec3(u_colors.displayGamma)), clr.w);
}