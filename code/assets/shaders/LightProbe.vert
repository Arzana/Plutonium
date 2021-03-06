#version 460 core

layout (push_constant) uniform PushConstants
{
	mat4 Model;
};

layout (location = 0) in vec3 Position;
layout (location = 1) in vec2 TexCoord;

layout (location = 0) out vec2 Uv;

void main()
{
	gl_Position = Model * vec4(Position, 1.0f);
	Uv = TexCoord;
}