#version 460 core

layout (push_constant) uniform Transforms
{
	mat4 Projection;
	mat4 View;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in mat4 Model;
layout (location = 5) in uint Color;

layout (location = 0) out vec4 VertexColor;

float unpack(in uint shift)
{
	return ((Color >> shift) & 0xFF) / 255.0f;
}

void main()
{
	// AABBGGRR
	const float r = unpack(0);
	const float g = unpack(8);
	const float b = unpack(16);

	// Blending is not enabled so just set alpha to one.
	gl_Position = Projection * View * Model * vec4(Position, 1.0f);
	VertexColor = vec4(r, g, b, 1.0f);
}