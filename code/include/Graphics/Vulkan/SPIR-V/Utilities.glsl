/*
	This file is included when using the GLSL to SPIR-V compiler in Plutonium.
	This file contains useful functions for any shader independent of it's type i.e. Vertex, Fragment, etc.
	All newly defined functions should be either purly mathematical or very global as we don't want to clutter the compiler.
	All newly defined functions should either be defined with "genType" if possible (float, double, sint, uint, bool, (d)vec2, (d)vec3, (d)vec4, (d)mat).
*/
#version 460 core
#extension GL_ARB_shading_language_include : require

/* Converts a single component value into an RGBA opaque color. */ 
vec4 opaque(in float value)
{
	return vec4(value, value, value, 1.0f);
}

/* Converts a single component value into an RGBA opaque color. */ 
dvec4 opaque(in double value)
{
	return dvec4(value, value, value, 1.0);
}

/* Converts individual RGB components into an RGBA opaque color. */
vec4 opaque(in float r, in float g, in float b)
{
	return vec4(r, g, b, 1.0f);
}

/* Converts individual RGB components into an RGBA opaque color. */
dvec4 opaque(in double r, in double g, in double b)
{
	return dvec4(r, g, b, 1.0);
}

/* Truncates the input value between zero and itself. */
float rectify(in float value)
{
	return max(0.0f, value);
}

/* Truncates the input value between zero and itself. */
double rectify(in double value)
{
	return max(0.0, value);
}

/* Truncates the input value between zero and itself. */
int rectify(in int value)
{
	return max(0, value);
}

/* Truncates the input value between zero and itself. */
vec2 rectify(in vec2 value)
{
	return vec2(rectify(value.x), rectify(value.y));
}

/* Truncates the input value between zero and itself. */
dvec2 rectify(in dvec2 value)
{
	return dvec2(rectify(value.x), rectify(value.y));
}

/* Truncates the input value between zero and itself. */
vec3 rectify(in vec3 value)
{
	return vec3(rectify(value.x), rectify(value.y), rectify(value.z));
}

/* Truncates the input value between zero and itself. */
dvec3 rectify(in dvec3 value)
{
	return dvec3(rectify(value.x), rectify(value.y), rectify(value.z));
}

/* Truncates the input value between zero and itself. */
vec4 rectify(in vec4 value)
{
	return vec4(rectify(value.x), rectify(value.y), rectify(value.z), rectify(value.w));
}

/* Truncates the input value between zero and itself. */
dvec4 rectify(in dvec4 value)
{
	return dvec4(rectify(value.x), rectify(value.y), rectify(value.z), rectify(value.w));
}

/* Truncates the value from zero to one. */
float saturate(in float value)
{
	return clamp(value, 0.0f, 1.0f);
}

/* Truncates the value from zero to one. */
double saturate(in double value)
{
	return clamp(value, 0.0, 1.0);
}

/* Truncates the value from zero to one. */
vec2 saturate(in vec2 value)
{
	return clamp(value, 0.0f, 1.0f);
}

/* Truncates the value from zero to one. */
dvec2 saturate(in dvec2 value)
{
	return clamp(value, 0.0, 1.0);
}

/* Truncates the value from zero to one. */
vec3 saturate(in vec3 value)
{
	return clamp(value, 0.0f, 1.0f);
}

/* Truncates the value from zero to one. */
dvec3 saturate(in dvec3 value)
{
	return clamp(value, 0.0f, 1.0f);
}

/* Truncates the value from zero to one. */
vec4 saturate(in vec4 value)
{
	return clamp(value, 0.0f, 1.0f);
}

/* Truncates the value from zero to one. */
dvec4 saturate(in dvec4 value)
{
	return clamp(value, 0.0, 1.0);
}

/* Reciprocates the input value. */ 
float recip(in float value)
{
	return 1.0f / value;
}

/* Reciprocates the input value. */ 
double recip(in double value)
{
	return 1.0 / value;
}

/* Reciprocates the input value. */ 
vec2 recip(in vec2 value)
{
	return vec2(1.0f) / value;
}

/* Reciprocates the input value. */ 
dvec2 recip(in dvec2 value)
{
	return dvec2(1.0) / value;
}

/* Reciprocates the input value. */ 
vec3 recip(in vec3 value)
{
	return vec3(1.0f) / value;
}

/* Reciprocates the input value. */ 
dvec3 recip(in dvec3 value)
{
	return dvec3(1.0f) / value;
}

/* Reciprocates the input value. */ 
vec4 recip(in vec4 value)
{
	return vec4(1.0f) / value;
}

/* Reciprocates the input value. */ 
dvec4 recip(in dvec4 value)
{
	return dvec4(1.0f) / value;
}

/* Converts from an RGB color to luminance. */
float clrToLuma(in vec3 color)
{
	return dot(color, vec3(0.229f, 0.587f, 0.114f));
}

/* Converts from HSV color space to sRGB color space. */
vec3 hsvToRgb(in float hue, in float saturation, in float value)
{
	const vec3 K = vec3(1.0f, 2.0f / 3.0f, 1.0f / 3.0f);
    vec3 p = abs(fract(vec3(hue) + K.xyz) * 6.0f - 3.0f);
    return value * mix(vec3(1.0f), saturate(p - 1.0f), saturation);
}

/* Converts from an RGB color to luminance. */
double clrToLuma(in dvec3 color)
{
	return dot(color, dvec3(0.229, 0.587, 0.114));
}

/* Gives the value of the quadratic function at the specified point x. */
float quadratic(in float a, in float b, in float c, in float x)
{
	return a * x * x + b * x + c;
}

/* Gives the value of the quadratic function at the specified point x. */
double quadratic(in double a, in double b, in double c, in double x)
{
	return a * x * x + b * x + c;
}