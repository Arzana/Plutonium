#version 460 core

layout (binding = 0) uniform UniformBuffer
{
	mat4 Projection;
};

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 TexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (location = 0) out vec2 Uv;

void main()
{
	gl_Position = Projection * vec4(Position, 0.0f, 1.0f);
	Uv = TexCoord;
}