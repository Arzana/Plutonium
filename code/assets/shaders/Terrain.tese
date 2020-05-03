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

mat3 sy = mat3( 
    1.0, 2.0, 1.0, 
    0.0, 0.0, 0.0, 
   -1.0, -2.0, -1.0);
mat3 sx = mat3( 
    1.0, 0.0, -1.0, 
    2.0, 0.0, -2.0, 
    1.0, 0.0, -1.0);

vec3 sobel(in vec2 uv, in float offset)
{
	// Construct sobel matrix.
	mat3 mat;
	mat[0][0] = textureOffset(Height, uv, ivec2(-1)).r;
	mat[0][1] = textureOffset(Height, uv, ivec2(0, -1)).r;
	mat[0][2] = textureOffset(Height, uv, ivec2(1, -1)).r;
	mat[1][0] = textureOffset(Height, uv, ivec2(-1, 0)).r;
	mat[1][1] = texture(Height, uv).r;
	mat[1][2] = textureOffset(Height, uv, ivec2(1, 0)).r;
	mat[2][0] = textureOffset(Height, uv, ivec2(-1, 1)).r;
	mat[2][1] = textureOffset(Height, uv, ivec2(0, 1)).r;
	mat[2][2] = textureOffset(Height, uv, ivec2(1)).r;
	mat *= Displacement;

	// Construct vertex normal from sobel samples.
	const float nx = dot(sx[0], mat[0]) + dot(sx[1], mat[1]) + dot(sx[2], mat[2]);
	const float nz = dot(sy[0], mat[0]) + dot(sy[1], mat[1]) + dot(sy[2], mat[2]);
	return normalize(vec3(nx * 2.0f, 1.0f, nz * 2.0f));
}

void main()
{
	// We need to sample at the center of the patch instead of in the corners.
	const float offset = 1.0f / (2.0f * PatchSize);

	// Set the first texture coordinate. 
	vec2 uv1 = mix(TexCoords1[0], TexCoords1[1], gl_TessCoord.x);
	vec2 uv2 = mix(TexCoords1[3], TexCoords1[2], gl_TessCoord.x);
	TexCoord1 = mix(uv1, uv2, gl_TessCoord.y);

	// Set the second texture coordinate. 
	uv1 = mix(TexCoords2[0], TexCoords2[1], gl_TessCoord.x);
	uv2 = mix(TexCoords2[3], TexCoords2[2], gl_TessCoord.x);
	TexCoord2 = mix(uv1, uv2, gl_TessCoord.y);

	// Calculate the vertex normal based on a sobel height filter.
	Normal = sobel(TexCoord2, offset);

	// Set the final position.
	const vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	const vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);
	pos.y += texture(Height, TexCoord2 + offset).r * Displacement;
	gl_Position = Projection * View * Model * pos;
}