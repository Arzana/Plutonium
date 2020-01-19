#version 460 core

layout (binding = 0) uniform Camera
{
	mat4 Projection;
	mat4 View;
};

layout (push_constant) uniform PushConstants
{
	mat4 Model;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec4 Tangent;
layout (location = 3) in vec2 TexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout (location = 0) out vec2 Uv;
layout (location = 1) out vec3 WorldPos;
layout (location = 2) out mat3 TBN;

void main()
{
	const vec3 t = normalize((Model * vec4(Tangent.xyz, 0.0f)).xyz);
	const vec3 n = normalize((Model * vec4(Normal, 0.0f)).xyz);
	const vec3 b = cross(n, t) * Tangent.w;
	TBN = mat3(t, b, n);

	Uv = TexCoord;

	const vec4 localPos = Model * vec4(Position, 1.0f);
	WorldPos = localPos.xyz;
	gl_Position = Projection * View * localPos;
}