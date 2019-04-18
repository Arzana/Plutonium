#version 460 core

layout (binding = 1, set = 1) uniform Specific
{
	vec4 Color;
	float Border;
	vec2 Size;
	vec2 Position;
};

layout (location = 0) in vec2 Uv;
layout (location = 0) out vec4 FragColor;

float RoundRect(in vec2 pos)
{
	const vec2 radius = Size / 2.0f - Border;
	return length(max(abs(pos) - radius, 0.0f)) - Border;
}

void main()
{
	const vec2 center = Position + Size / 2.0f;
	if (RoundRect(gl_FragCoord.xy - center) <= 0.0f)
	{
		FragColor = Color;
	}
	else discard;
}