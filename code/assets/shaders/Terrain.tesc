#version 460 core
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_tessellation_shader : require
layout (vertices = 4) out;
layout (constant_id = 0) const float MaxTessellation = 64.0f;

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

layout (location = 0) in vec3 Normals[];
layout (location = 1) in vec2 TexCoords1[];
layout (location = 2) in vec2 TexCoords2[];

layout (location = 0) out vec3 WorldNormals[4];
layout (location = 1) out vec2 Uvs1[4];
layout (location = 2) out vec2 Uvs2[4];

// Calculates the tessellation factor based on scrren space dimensions of the edge.
float screenSpaceTessellationFactor(in vec4 p, in vec4 q)
{
	// Move the edge midpoint to view space and get the radius between the control point.
	const float r = distance(p, q) * 0.5f;
	const vec4 adder = vec4(r, vec3(0.0f));
	const vec4 v = View * Model * (0.5f * (p + q));

	// Project view midpoint into NDC.
	vec4 clip0 = Projection * (v - adder);
	vec4 clip1 = Projection * (v + adder);
	clip0 /= clip0.w;
	clip1 /= clip1.w;

	// Scale to viewport dimensions.
	clip0.xy *= Viewport;
	clip1.xy *= Viewport;

	// We clamp between [1, MaxTessellation] range.
	// Level = 1 of tessellation means no tessellation, so it has no use to go below this.
	// Level = MaxTessellation. At this point we gain no more precision from sampling the heightmap.
	return clamp(distance(clip0, clip1) / EdgeSize * Tessellation, 1.0f, MaxTessellation);
}

bool cull()
{
	vec4 pos = gl_in[gl_InvocationID].gl_Position;
	pos.y += imageLoad(Height, ivec2(TexCoords1[0])).r * Displacement;
	pos = Model * pos;

	// Check the sphere against the frustum planes.
	// The culling radius is either the PatchSize (default)
	// or the Displacement (if it's greater than the PatchSize).
	const float iradius = -max(Displacement, PatchSize);
	for (uint i = 0; i < 6; i++)
	{
		if (dot(pos, Frustum[i]) < iradius) return true;
	}

	// Sphere is on the correct side of all the planes, so don't cull.
	return false;
}

void main()
{
	if (gl_InvocationID == 0)
	{
		if (cull())
		{
			// The vertex isn't visible so just don't generate a patch.
			gl_TessLevelOuter[0] = 0.0f;
			gl_TessLevelOuter[1] = 0.0f;
			gl_TessLevelOuter[2] = 0.0f;
			gl_TessLevelOuter[3] = 0.0f;

			gl_TessLevelInner[0] = 0.0f;
			gl_TessLevelInner[1] = 0.0f;
		}
		else if (Tessellation > 0.0f)
		{
			// Set the outer factors based on their screen space distance.
			gl_TessLevelOuter[0] = screenSpaceTessellationFactor(gl_in[3].gl_Position, gl_in[0].gl_Position);
			gl_TessLevelOuter[1] = screenSpaceTessellationFactor(gl_in[0].gl_Position, gl_in[1].gl_Position);
			gl_TessLevelOuter[2] = screenSpaceTessellationFactor(gl_in[1].gl_Position, gl_in[2].gl_Position);
			gl_TessLevelOuter[3] = screenSpaceTessellationFactor(gl_in[2].gl_Position, gl_in[3].gl_Position);

			// Set the inner factors to the average of the two outer factors (linear scaling).
			gl_TessLevelInner[0] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5f);
			gl_TessLevelInner[1] = mix(gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5f);
		}
		else
		{
			// Simple passthrough.
			gl_TessLevelOuter[0] = 1.0f;
			gl_TessLevelOuter[1] = 1.0f;
			gl_TessLevelOuter[2] = 1.0f;
			gl_TessLevelOuter[3] = 1.0f;

			gl_TessLevelInner[0] = 1.0f;
			gl_TessLevelInner[1] = 1.0f;
		}
	}

	// Pass through position and texture coordinates.
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	WorldNormals[gl_InvocationID] = Normals[gl_InvocationID];
	Uvs1[gl_InvocationID] = TexCoords1[gl_InvocationID];
	Uvs2[gl_InvocationID] = TexCoords2[gl_InvocationID];
}