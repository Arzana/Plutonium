#pragma once
#include <glad\glad.h>

namespace Plutonium
{
	/* Specifies the types of OpenGL shaders supported by this framework. */
	enum class ShaderType : GLenum
	{
		/* An invalid or unknown shader type. */
		Invalid = 0,
		/* A vertex shader. */
		Vertex = GL_VERTEX_SHADER,
		/* A geometry shader. */
		Geometry = GL_GEOMETRY_SHADER,
		/* A fragment shader. */
		Fragment = GL_FRAGMENT_SHADER
	};

	/* Gets a string version of a specified OpenGL shader type. */
	_Check_return_ inline const char* _CrtGetShaderVisualType(_In_ ShaderType type)
	{
		switch (type)
		{
			case Plutonium::ShaderType::Vertex:
				return "Vertex";
			case Plutonium::ShaderType::Geometry:
				return "Geometry";
			case Plutonium::ShaderType::Fragment:
				return "Fragment";
			default:
				return "UNKNOWN";
		}
	}
}