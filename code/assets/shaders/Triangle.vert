#version 460 core

void main()
{
	const vec2 pos[3] = vec2[3](vec2(-0.7f, 0.7f), vec2(0.7f, 0.7f), vec2(0.0f, -0.7f));
	gl_Position = vec4(pos[gl_VertexIndex], 0.0f, 1.0f);
}