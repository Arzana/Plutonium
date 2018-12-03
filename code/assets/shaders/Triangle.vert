#version 460 core

layout (location = 0) out vec4 VertexColor; 

void main()
{
	const vec2 pos[3] = vec2[3](vec2(-0.7f, 0.7f), vec2(0.7f, 0.7f), vec2(0.0f, -0.7f));
	const vec4 clr[3] = vec4[3](vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f));

	VertexColor = clr[gl_VertexIndex];
	gl_Position = vec4(pos[gl_VertexIndex], 0.0f, 1.0f);
}