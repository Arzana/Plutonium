#version 460 core

layout (binding = 0) uniform Transforms
{
	mat4 Projection;
	mat4 View;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in uint Color;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (location = 0) out vec4 VertexColor;

void main()
{
	const float r = (Color & 0xFF000000) / 255.0f;
	const float g = (Color & 0x00FF0000) / 255.0f;
	const float b = (Color & 0x0000FF00) / 255.0f;

	gl_Position = Projection * View * vec4(Position, 1.0f);
	VertexColor = vec4(r, g, b, 1.0f);
}