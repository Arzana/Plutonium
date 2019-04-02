#version 460 core

layout (binding = 0) uniform Transforms
{
	mat4 Projection;
	mat4 View;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (location = 0) out vec3 FragNormal;

void main()
{
	gl_Position = Projection * View * vec4(Position, 1.0f);
	FragNormal = Normal;
}