#version 460 core

layout (binding = 0) uniform Transforms
{
	mat4 Projection;
	mat4 View;
	mat4 Model;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (location = 0) out vec2 Uv;
layout (location = 1) out vec3 VertexNormal;
layout (location = 2) out vec3 WorldPos;

void main()
{
	gl_Position = Projection * View * Model * vec4(Position, 1.0f);

	Uv = TexCoord;
	VertexNormal = Normal;
	WorldPos = Position;
}