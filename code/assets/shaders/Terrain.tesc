#version 460 core
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_tessellation_shader : require
layout (vertices = 4) out;

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

layout (location = 0) out vec3 WorldNormals[4];
layout (location = 1) out vec2 Uvs[4];

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

	return clamp(distance(clip0, clip1) / EdgeSize * Tessellation, 1.0f, 64.0f);
}

bool cull()
{
	const float r = 8.0f; //TODO: change this.

	vec4 pos = gl_in[gl_InvocationID].gl_Position;
	pos.y += texture(Height, TexCoords[0]).r * Displacement;

	// Check the sphere against the frustum planes.
	for (uint i = 0; i < 6; i++)
	{
		if (dot(pos, Frustum[i]) + r < 0.0f) return true;
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
		else
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
	}

	// Pass through position, normal and texture coordinates.
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	WorldNormals[gl_InvocationID] = Normals[gl_InvocationID];
	Uvs[gl_InvocationID] = TexCoords[gl_InvocationID];
}