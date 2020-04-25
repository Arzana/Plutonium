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

layout (location = 0) out vec2 Uv;
layout (location = 1) out mat3 TBN;

void main()
{
	// Set the position.
	gl_Position = Projection * View * Model * vec4(Position, 1.0f);

	// Set the texture coordinate.
	Uv = TexCoord;

	// Set the bump-mapped normal.
	const mat3 scaled = mat3(transpose(inverse(Model)));
	const vec3 t = normalize(scaled * Tangent.xyz.xyz);
	const vec3 n = normalize(scaled * Normal.xyz);
	const vec3 b = cross(n, t) * Tangent.w;
	TBN = mat3(t, b, n);
}