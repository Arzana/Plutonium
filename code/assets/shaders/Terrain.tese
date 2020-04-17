#version 460 core
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_tessellation_shader : require
layout (quads, equal_spacing, ccw) in;

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
	float PatchSize;
};

layout (location = 0) in vec2 TexCoords1[];
layout (location = 1) in vec2 TexCoords2[];

layout (location = 0) out vec2 TexCoord1;
layout (location = 1) out vec2 TexCoord2;
layout (location = 2) out vec3 Normal;

void main()
{
	// Set the first texture coordinate. 
	vec2 uv1 = mix(TexCoords1[0], TexCoords1[1], gl_TessCoord.x);
	vec2 uv2 = mix(TexCoords1[3], TexCoords1[2], gl_TessCoord.x);
	TexCoord1 = mix(uv1, uv2, gl_TessCoord.y);

	// Set the second texture coordinate. 
	uv1 = mix(TexCoords2[0], TexCoords2[1], gl_TessCoord.x);
	uv2 = mix(TexCoords2[3], TexCoords2[2], gl_TessCoord.x);
	TexCoord2 = mix(uv1, uv2, gl_TessCoord.y);

	Normal = vec3(0.0f, 1.0f, 0.0f);

	// Set the final position.
	const vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	const vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);
	pos.y += texture(Height, TexCoord2).r * Displacement;
	gl_Position = Projection * View * Model * pos;
}