#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) out vec2 Uv;

void main()
{
	Uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(Uv * 2.0f - 1.0f, 0.0f, 1.0f);
}