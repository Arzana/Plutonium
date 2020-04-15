#version 460 core
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_tessellation_shader : require
layout (quads, equal_spacing, cw) in;

layout (binding = 0) uniform Camera
{
	mat4 Projection;
	mat4 View;
	vec4 Frustum[6];
	vec2 Viewport;
};

layout (set = 1, binding = 0) uniform sampler2D Height;
layout (set = 1, binding = 3) uniform Terrain
{
	mat4 Model;
	float Displacement;
	float Tessellation;
	float EdgeSize;
};

layout (location = 0) in vec3 Normals[];
layout (location = 1) in vec2 TexCoords[];

layout (location = 0) out vec3 Normal;
layout (location = 1) out vec2 TexCoord;

void main()
{
	// Set the final texture coordinate. 
	const vec2 uv1 = mix(TexCoords[0], TexCoords[1], gl_TessCoord.x);
	const vec2 uv2 = mix(TexCoords[3], TexCoords[2], gl_TessCoord.x);
	TexCoord = mix(uv1, uv2, gl_TessCoord.y);

	// Set the final normal.
	const vec3 n1 = mix(Normals[0], Normals[1], gl_TessCoord.x);
	const vec3 n2 = mix(Normals[3], Normals[2], gl_TessCoord.x);
	Normal = mix(n1, n2, gl_TessCoord.y);

	// Set the final position.
	const vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	const vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);
	pos.y += texture(Height, TexCoord).r * Displacement;
	gl_Position = Projection * View * Model * pos;
}