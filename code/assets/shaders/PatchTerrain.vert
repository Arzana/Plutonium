#version 460 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord1;
layout (location = 3) in vec2 TexCoord2;

layout (location = 0) out vec3 WorldNormal;
layout (location = 1) out vec2 Uv1;;
layout (location = 2) out vec2 Uv2;

void main()
{
	gl_Position = vec4(Position, 1.0f);
	WorldNormal = Normal;
	Uv1 = TexCoord1;
	Uv2 = TexCoord2;
}