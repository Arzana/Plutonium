#version 460 core

layout (location = 0) in vec3 FragNormal;
layout (location = 0) out vec4 FragColor;

void main()
{
	vec3 diffuse = vec3(0.5f);
	vec3 lightDir = vec3(0.5f, -0.5f, 0.0f);
	float ambientFactor = 0.2f;

	FragColor = vec4(diffuse * max(ambientFactor, dot(FragNormal, lightDir)), 1.0f);
}