#include "Graphics\Rendering\Shader.h"
#include "Core\StringFunctions.h"
#include "Streams\FileReader.h"
#include "Core\SafeMemory.h"
#include "Core\EnumUtils.h"

using namespace Plutonium;

Plutonium::Shader::Shader(void)
	: ptr(0), loaded(false), beginCalled(false)
{}

Plutonium::Shader::Shader(const char * vertexShaderSource)
	: Shader()
{
	Initialize(vertexShaderSource, nullptr, nullptr);
}

Plutonium::Shader::Shader(const char * vertexShaderSource, const char * fragmentShaderSource)
	: Shader()
{
	Initialize(vertexShaderSource, nullptr, fragmentShaderSource);
}

Plutonium::Shader::Shader(const char * vertexShaderSource, const char * geometryShaderSource, const char * fragmentShaderSource)
	: Shader()
{
	Initialize(vertexShaderSource, geometryShaderSource, fragmentShaderSource);
}

Plutonium::Shader::~Shader(void)
{
	/* If shader was created delete it. */
	if (ptr)
	{
		glDeleteProgram(ptr);
		ptr = 0;
	}

	/* Delete created fields. */
	while (fields.size() > 0)
	{
		Field *cur = fields.back();
		free_s(cur->name);
		delete_s(cur);
		fields.pop_back();
	}
}

Shader * Plutonium::Shader::FromFile(const char * vertexShaderPath)
{
	/* Get content of shader files. */
	const char *vrtxShdr = FileReader(vertexShaderPath).ReadToEnd();

	/* Create result. */
	Shader *result = new Shader(vrtxShdr);

	/* Release source strings. */
	free_s(vrtxShdr);

	/* Return shader. */
	return result;
}

Shader * Plutonium::Shader::FromFile(const char * vertexShaderPath, const char * fragmentShaderPath)
{
	/* Get content of shader files. */
	const char *vrtxShdr = FileReader(vertexShaderPath).ReadToEnd();
	const char *fragShdr = FileReader(fragmentShaderPath).ReadToEnd();

	/* Create result. */
	Shader *result = new Shader(vrtxShdr, fragShdr);

	/* Release source strings. */
	free_s(vrtxShdr);
	free_s(fragShdr);

	/* Return shader. */
	return result;
}

Shader * Plutonium::Shader::FromFile(const char * vertexShaderPath, const char * geometryShaderPath, const char * fragmentShaderPath)
{
	/* Get content of shader files. */
	const char *vrtxShdr = FileReader(vertexShaderPath).ReadToEnd();
	const char *geomShdr = FileReader(geometryShaderPath).ReadToEnd();
	const char *fragShdr = FileReader(fragmentShaderPath).ReadToEnd();

	/* Create result. */
	Shader *result = new Shader(vrtxShdr, geomShdr, fragShdr);

	/* Release source strings. */
	free_s(vrtxShdr);
	free_s(geomShdr);
	free_s(fragShdr);

	/* Return shader. */
	return result;
}

Uniform * Plutonium::Shader::GetUniform(const char * name) const
{
	/* Loop through fields. */
	for (size_t i = 0; i < fields.size(); i++)
	{
		/* If name matches return as uniform. */
		Field *cur = fields.at(i);
		if (!strcmp(cur->name, name)) return static_cast<Uniform*>(cur);
	}

	ASSERT("Cannot find uniform '%s'!", name);
	return nullptr;
}

Attribute * Plutonium::Shader::GetAttribute(const char * name) const
{
	/* Loop through fields. */
	for (size_t i = 0; i < fields.size(); i++)
	{
		/* If name matches return as attribute. */
		Field *cur = fields.at(i);
		if (!strcmp(cur->name, name)) return static_cast<Attribute*>(cur);
	}

	ASSERT("Cannot find attribute %s!", name);
	return nullptr;
}

void Plutonium::Shader::Begin(void)
{
	/* On debug check if the shader is propperly loaded. */
	ASSERT_IF(!loaded, "Cannot start failed shader program!");

	/* Start use of shader or log warning. */
	if (!beginCalled)
	{
		/* Start shader. */
		beginCalled = true;
		glUseProgram(ptr);

		/* Enable attribute arrays. */
		for (size_t i = 0; i < fields.size(); i++)
		{
			Field *cur = fields.at(i);
			if (cur->IsAttribute()) static_cast<Attribute*>(cur)->Enable();
		}
	}
	else LOG_WAR("Attempting to start already started shader!");
}

void Plutonium::Shader::End(void)
{
	/* On debug check if the shader is propperly loaded. */
	ASSERT_IF(!loaded, "Cannot end failed shader program!");

	/* End use of shader or log warning. */
	if (beginCalled)
	{
		/* End shader. */
		beginCalled = false;

		/* Disable attribute arrays. */
		for (size_t i = 0; i < fields.size(); i++)
		{
			Field *cur = fields.at(i);
			if (cur->IsAttribute()) static_cast<Attribute*>(cur)->Disable();
		}
	}
	else LOG_WAR("Attempting to end not started shader!");
}

bool Plutonium::Shader::CompileShader(uint32 * shdr, ShaderType type, const char * src)
{
	/* Check if source isn't null or empty. */
	if (!src || strlen(src) < 1)
	{
		LOG_THROW_IF(!src || strlen(src) < 1, "Cannot compile %s shader from empty source!", _CrtGetShaderVisualType(type));
		return false;
	}

	/* Create and compile shader. */
	*shdr = glCreateShader(_CrtEnum2Int(type));
	glShaderSource(*shdr, 1, &src, nullptr);
	glCompileShader(*shdr);

	/* On debug mode check shader log and report if needed. */
#if defined(DEBUG)
	/* Get compile log length. */
	int32 len;
	glGetShaderiv(*shdr, GL_INFO_LOG_LENGTH, &len);
	if (len > 1)
	{
		/* Log is available so log it to output. */
		char *log = malloc_s(char, len);
		glGetShaderInfoLog(*shdr, len, &len, log);
		LOG_WAR("Shader compilation log:\n%s", log);
		free_s(log);
	}
#endif

	/* Get whether the shader is successfuly compiled. */
	int32 state;
	glGetShaderiv(*shdr, GL_COMPILE_STATUS, &state);
	if (!state)
	{
		/* Throw exception with source. */
		LOG_THROW("Failed to compile %s shader!\nSOURCE:\n%s", _CrtGetShaderVisualType(type), src);
		return false;
	}

	/* On debug log shader compile success. */
	LOG("Successfuly compiled %s shader.", _CrtGetShaderVisualType(type));
	return true;
}

void Plutonium::Shader::LoadFields(void)
{
	/* Load the uniforms and the attributes individuals to throw better exceptions. */
	LoadUniforms();
	LoadAttributes();
}

void Plutonium::Shader::LoadUniforms(void)
{
	/* Get the amount of uniforms defined by the shader. */
	int32 uniformCnt;
	glGetProgramiv(ptr, GL_ACTIVE_UNIFORMS, &uniformCnt);

	/* Get the maximum name length of the uniforms and create a buffer that can hold that size. */
	int32 nameBufferSize;
	glGetProgramiv(ptr, GL_ACTIVE_UNIFORM_MAX_LENGTH, &nameBufferSize);
	char *buffer = malloca_s(char, nameBufferSize);

	/* Loop through all uniforms. */
	for (int32 uniform = 0, samplerCnt = 0; uniform < uniformCnt; uniform++)
	{
		/* Get raw uniform information. */
		int32 dimensions;
		uint32 type;
		int32 nameLen;
		glGetActiveUniform(ptr, uniform, nameBufferSize, &nameLen, &dimensions, &type, buffer);

		/* Get uniform name sub string. */
		char *name = malloc_s(char, nameLen + 1);
		substr(buffer, 0, nameLen, name);
		name[nameLen] = '\0';

		/* Add uniform to the field list. */
		int32 curSampler = type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE ? samplerCnt++ : 0;
		fields.push_back(new Uniform(glGetUniformLocation(ptr, name), name, type, curSampler));
	}

	freea_s(buffer);
}

void Plutonium::Shader::LoadAttributes(void)
{
	/* Get the amount of attributes defined by the shader. */
	int32 attribCnt;
	glGetProgramiv(ptr, GL_ACTIVE_ATTRIBUTES, &attribCnt);

	/* The the maximum name length of the attributes and create a buffer that can hold that size. */
	int32 nameBufferSize;
	glGetProgramiv(ptr, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &nameBufferSize);
	char *buffer = malloca_s(char, nameBufferSize);

	/* Loop through all attributes. */
	for (int32 attrib = 0; attrib < attribCnt; attrib++)
	{
		/* Get raw attribute information. */
		int32 dimensions;
		uint32 type;
		int32 nameLen;
		glGetActiveAttrib(ptr, attrib, nameBufferSize, &nameLen, &dimensions, &type, buffer);

		/*Get atribute name sub string. */
		char *name = malloc_s(char, nameLen + 1);
		substr(buffer, 0, nameLen, name);
		name[nameLen] = '\0';

		/* Add attribute to the field list. */
		fields.push_back(new Attribute(glGetAttribLocation(ptr, name), name, type));
	}

	freea_s(buffer);
}

void Plutonium::Shader::Initialize(const char * vrtxShdr, const char * geomShdr, const char * fragShdr)
{
	/* Create new shader program. */
	ptr = glCreateProgram();

	/* Compile vertex shader. */
	uint32 vrtxShdrPtr;
	if (CompileShader(&vrtxShdrPtr, ShaderType::Vertex, vrtxShdr)) AddShader(vrtxShdrPtr);

	/* Compile geometry shader (if present). */
	uint32 geomShdrPtr = 0;
	if (geomShdr)
	{
		if (CompileShader(&geomShdrPtr, ShaderType::Geometry, geomShdr)) AddShader(geomShdrPtr);
	}

	/* Compile fragment shader (if present). */
	uint32 fragShdrPtr = 0;
	if (fragShdr)
	{
		if (CompileShader(&fragShdrPtr, ShaderType::Fragment, fragShdr)) AddShader(fragShdrPtr);
	}

	/* Links to program to OpenGL. */
	if (!LinkProgram())
	{
		/* Delete temporary shaders. */
		glDeleteShader(vrtxShdrPtr);
		if (geomShdr) glDeleteShader(geomShdrPtr);
		if (fragShdr) glDeleteShader(fragShdrPtr);

		/* Link has failed, delete program. */
		glDeleteProgram(ptr);
		ptr = 0;
		return;
	}

	/* Delete temporary shaders. */
	glDeleteShader(vrtxShdrPtr);
	if (geomShdr) glDeleteShader(geomShdrPtr);
	if (fragShdr) glDeleteShader(fragShdrPtr);

	/* Load the fields of the current shader. */
	LoadFields();
	loaded = true;
}

bool Plutonium::Shader::LinkProgram(void)
{
	/* Link shader program. */
	glLinkProgram(ptr);

	/* Get the link state of the shader program. */
	int32 state;
	glGetProgramiv(ptr, GL_LINK_STATUS, &state);
	if (!state)
	{
		/* On debug mode get the link log. */
#if defined(DEBUG)
		int32 logLen;
		glGetProgramiv(ptr, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 1)
		{
			/* Log is availabe so log it to the output. */
			char *log = malloc_s(char, logLen);
			glGetProgramInfoLog(ptr, logLen, &logLen, log);
			LOG_WAR("Shader Program link log:\n%s", log);
			free_s(log);
		}
#endif

		LOG_THROW("Failed to link shader program!");
		return false;
	}

	/* On debug log shader link success. */
	LOG("Successfuly linked program.");
	return true;
}

void Plutonium::Shader::AddShader(uint32 shdr)
{
	glAttachShader(ptr, shdr);
}