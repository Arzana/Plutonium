#version 460 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec4 Color;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (location = 0) out vec4 VertexColor;

void main()
{
	gl_Position = vec4(Position, 1.0f);
	VertexColor = Color;
}