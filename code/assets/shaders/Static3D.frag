#version 430 core

// Structures.
struct LightInfo
{
	vec3 reflectedDirection;
	float diffuseIntensity;
};

struct ObjectMaps
{
	sampler2D ambient;
	sampler2D diffuse;
	sampler2D specular;
	sampler2D alpha;
};

struct ObjectColors
{
	vec4 lfilter;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float specularExponent;
};

struct Light
{
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

// Uniforms.
uniform ObjectMaps u_textures;
uniform ObjectColors u_colors;
uniform Light u_light_sun;
uniform PLight u_light_vases[4];
uniform vec3 u_view_pos;

// Inputs.
in LightInfo a_light;
in vec3 a_vrtx_pos;
in vec2 a_texture;

// Outputs.
out vec4 fragColor;

// Calculates the color change for the object from a specified directional light source.
vec4 CalcDirectionalLight(Light light)
{
	float power = pow(max(dot(normalize(u_view_pos - a_vrtx_pos), a_light.reflectedDirection), 0.0f), u_colors.specularExponent);

	vec4 ambient = texture(u_textures.ambient, a_texture) * u_colors.ambient * light.ambient;
	vec4 diffuse = texture(u_textures.diffuse, a_texture) * u_colors.diffuse * a_light.diffuseIntensity * light.diffuse;
	vec4 specular = texture(u_textures.specular, a_texture) * u_colors.specular * power * light.specular;

	return ambient + diffuse + specular;
}

// Calculates the color change for the object from a specified point light source.
vec4 CalcPointLight(PLight light)
{
	float distance = length(light.position - a_vrtx_pos);
	float attenuation = 1.0f / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
	float power = pow(max(dot(normalize(u_view_pos - a_vrtx_pos), a_light.reflectedDirection), 0.0f), u_colors.specularExponent);

	vec4 ambient = texture(u_textures.ambient, a_texture) * u_colors.ambient * light.ambient * attenuation;
	vec4 diffuse = texture(u_textures.diffuse, a_texture) * u_colors.diffuse * a_light.diffuseIntensity * light.diffuse * attenuation;
	vec4 specular = texture(u_textures.specular, a_texture) * u_colors.specular * power * light.specular * attenuation;

	return ambient + diffuse + specular;
}

void main()
{
	// Calculate fragment filter and discard fragment if invisible.
	vec4 modifier = u_colors.lfilter * texture(u_textures.alpha, a_texture);
	if ((modifier.r < 0.1f && modifier.g < 0.1f && modifier.b < 0.1f) || modifier.a < 0.1f) discard;

	// Apply lighting.
	vec4 outputColor = CalcDirectionalLight(u_light_sun);
	for (int i = 0; i < u_light_vases.length(); i++) outputColor += CalcPointLight(u_light_vases[i]);
	
	// Calculate fragment's final color.  
	fragColor = outputColor * modifier;
}