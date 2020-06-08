#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0) uniform Camera
{
	mat4 Projection;
	mat4 View;
	vec4 Frustum[6];
	vec2 Viewport;
};

layout (set = 1, binding = 0, r32f) uniform image2D Height;
layout (set = 1, binding = 3) uniform Terrain
{
	mat4 Model;
	float Displacement;
	float Tessellation;
	float EdgeSize;
	float PatchSize;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord1;
layout (location = 3) in vec2 TexCoord2;

layout (location = 0) out vec3 WorldNormal;
layout (location = 1) out vec2 Uv1;;
layout (location = 2) out vec2 Uv2;

void main()
{
	const vec4 pos = vec4(Position.x, imageLoad(Height, ivec2(TexCoord1)).r * Displacement, Position.z, 1.0f);
	gl_Position = Projection * View * Model * pos;
	WorldNormal = Normal;
	Uv1 = TexCoord1;
	Uv2 = TexCoord2;
}