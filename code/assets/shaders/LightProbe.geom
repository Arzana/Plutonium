#version 460 core
layout (triangles, invocations = 6) in;
layout (triangle_strip, max_vertices = 3) out;

layout (binding = 0) uniform VPs
{
	mat4 Transforms[6];
};

layout (location = 0) in vec2 Uv[];

layout (location = 0) out vec2 FragUv;

void main()
{
	for (uint i = 0; i < gl_in.length(); i++)
	{
		gl_Position = Transforms[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer = gl_InvocationID;
		FragUv = Uv[i];
		EmitVertex();
	}

	EndPrimitive();
}