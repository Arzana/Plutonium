#version 460 core

const float HeightScalar = 10.0f;

layout (binding = 0) uniform Transforms
{
	mat4 Projection;
	mat4 View;
	mat4 Model;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (location = 0) out vec3 WorldPos;
layout (location = 1) out vec3 VertexNormal;
layout (location = 2) out float Height;

void main()
{
	const vec4 localPos = Model * vec4(Position * vec3(1.0f, HeightScalar, 1.0f), 1.0f);

	WorldPos = localPos.xyz;
	VertexNormal = normalize((Model * vec4(Normal, 0.0f)).xyz);
	Height = 1.0f - Position.y;	// The projection matrix inverts the height
	
	gl_Position = Projection * View * localPos;
}