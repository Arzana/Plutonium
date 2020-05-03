#version 460 core

layout (binding = 0) uniform Camera
{
	mat4 Projection;
	mat4 View;
};

layout (push_constant) uniform PushConstants
{
	mat4 Model;
	float Blending;
};

layout (location = 0) in vec3 Position1;
layout (location = 1) in vec3 Normal1;
layout (location = 2) in vec2 TexCoord1;

layout (location = 3) in vec3 Position2;
layout (location = 4) in vec3 Normal2;
layout (location = 5) in vec2 TexCoord2;

layout (location = 0) out vec3 WorldNormal;
layout (location = 1) out vec2 Uv;

void main()
{
	// Set the position.
	const vec3 pos = mix(Position1, Position2, Blending);
	gl_Position = Projection * View * Model * vec4(pos, 1.0f);

	// Set the normal.
	const vec3 norm = mix(Normal1, Normal2, Blending);
	WorldNormal = normalize(mat3(transpose(inverse(Model))) * norm);

	// Set texture coordinate.
	Uv = mix(TexCoord1, TexCoord2, Blending);
}