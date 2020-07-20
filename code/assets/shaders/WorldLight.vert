#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0) uniform Camera
{
	mat4 Projection;
	mat4 View;
};

layout (location = 0) in vec3 Position;

layout (location = 1) in vec2 InstanceAttenuation;
layout (location = 2) in vec4 InstanceRadiance;
layout (location = 3) in mat4 InstanceVolume;

layout (location = 0) out vec3 LightPosition;
layout (location = 1) out vec2 LightAttenuation;
layout (location = 2) out vec4 LightRadiance;

void main()
{
	// The light's position is part of the volume matrix.
	const vec3 pos = vec3(InstanceVolume[3][0], InstanceVolume[3][1], InstanceVolume[3][2]);

	LightPosition = pos;
	LightAttenuation = InstanceAttenuation;
	LightRadiance = InstanceRadiance;

	gl_Position = Projection * View * InstanceVolume * vec4(Position, 1.0f);
}