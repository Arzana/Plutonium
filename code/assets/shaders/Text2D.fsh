#version 430 core

uniform sampler2D u_texture;
uniform vec4 u_color;

in vec2 v_texture;

out vec4 fragColor;

void main()
{
	fragColor = u_color * texture(u_texture, v_texture);
}