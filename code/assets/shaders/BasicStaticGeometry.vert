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
layout (location = 2) in vec2 TexCoord;

layout (location = 0) out vec3 WorldNormal;
layout (location = 1) out vec2 Uv;

void main()
{
	// Set the position.
	gl_Position = Projection * View * Model * vec4(Position, 1.0f);

	// Set the primitive properties.
	WorldNormal = Normal;
	Uv = TexCoord;
}