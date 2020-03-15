#version 460 core
#extension GL_KHR_vulkan_glsl : enable

layout (binding = 0) uniform Camera
{
	mat4 IProjection;
	mat4 IView;
};

layout (location = 0) out vec3 Angle;

void main()
{
	// Always render the skybox at the far end of the depth buffer
	const vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(uv * 2.0f - 1.0f, 1.0f, 1.0f);

	// The viewing angle can be easily calculated using the inverse matrices.
	const vec4 eye = IProjection * gl_Position;
	Angle = (IView * (eye / eye.w)).xyz;
}