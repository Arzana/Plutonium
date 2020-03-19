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

layout (location = 0) out vec3 WorldPos;
layout (location = 1) out vec2 Uv;
layout (location = 2) out mat3 TBN;

void main()
{
	// Set the position.
	const vec4 pos = Model * vec4(Position, 1.0f);
	WorldPos = pos.xyz;
	gl_Position = Projection * View * pos;

	// Set the texture coordinate.
	Uv = TexCoord;

	// Set the bump-mapped normal.
	const vec3 t = normalize((Model * vec4(Tangent.xyz, 0.0f)).xyz);
	const vec3 n = normalize((Model * vec4(Normal, 0.0f)).xyz);
	const vec3 b = cross(n, t) * Tangent.w;
	TBN = mat3(t, b, n);
}