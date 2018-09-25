#pragma once
#include "Attribute.h"
#include "Uniform.h"
#include "ShaderTypes.h"
#include <vector>

namespace Plutonium
{
	/* Defines a helper object for OpenGL shaders. */
	class Shader
	{
	public:
		/* Initializes a new instance of a shader with only a vertex shader. */
		Shader(_In_ const char *vertexShaderSource);
		/* Initializes a new instance of a shader with a vertex and fragment shader. */
		Shader(_In_ const char *vertexShaderSource, _In_ const char *fragmentShaderSource);
		/* Initializes a new instance of a shader with all three types of shaders. */
		Shader(_In_ const char *vertexShaderSource, _In_ const char *geometryShaderSource, _In_ const char *fragmentShaderSource);
		Shader(_In_ const Shader &value) = delete;
		Shader(_In_ Shader &&value) = delete;
		/* Releases the resources allocated by the shader. */
		~Shader(void);

		_Check_return_ Shader& operator =(_In_ const Shader &other) = delete;
		_Check_return_ Shader& operator = (_In_ Shader &&other) = delete;

		/* Initializes a new instance of a shader with only a vertex shader from the specified file source. */
		_Check_return_ static Shader* FromFile(_In_ const char *vertexShaderPath);
		/* Initializes a new instance of a shader with a vertex and fragment shaders from the specified file sources. */
		_Check_return_ static Shader* FromFile(_In_ const char *vertexShaderPath, _In_ const char *fragmentShaderPath);
		/* Initializes a new instance of a shader with all three types of shaders from the specified file sources. */
		_Check_return_ static Shader* FromFile(_In_ const char *vertexShaderPath, _In_ const char *geometryShaderPath, _In_ const char *fragmentShaderPath);

		/* Gets a uniform defined by the shader, if non is found matching the name; return nullptr. */
		_Check_return_ Uniform* GetUniform(_In_ const char *name) const;
		/* Gets a attribute defined by the shader, if non is found matching the name; return nullptr. */
		_Check_return_ Attribute* GetAttribute(_In_ const char *name) const;
		/* Gets whether the shader was successfuly initialized. */
		_Check_return_ inline bool IsValid(void) const
		{
			return loaded;
		}

		/* Starts the use if this shader. */
		void Begin(void);
		/* Ends the use of this shader. */
		void End(void);

	private:
		uint32 ptr;
		bool loaded, beginCalled;
		std::vector<Field*> fields;

		Shader(void);

		static bool CompileShader(uint32 *shdr, ShaderType type, const char *src);

		void LoadFields(void);
		void LoadUniforms(void);
		void LoadAttributes(void);

		void Initialize(const char *vrtxShdr, const char *geomShdr, const char *fragShdr);
		bool LinkProgram(void);
		void AddShader(uint32 shdr);
		std::string GetShaderSource(void) const;
	};
}