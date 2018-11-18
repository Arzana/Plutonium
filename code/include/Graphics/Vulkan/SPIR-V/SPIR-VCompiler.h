#pragma once
#include "Core/String.h"

namespace Pu
{
	/* Helper object to convert from high level languages to SPIR-V. */
	class SPIRV
	{
	public:
		/* Converts from GLSL source code to a SPIR-V source. */
		_Check_return_ static string FromGLSL(_In_ const string &src);
		/* Converts from a GLSL code file to SPIR-V source. */
		_Check_return_ static string FromGLSLPath(_In_ const string &path);

	private:
		static constexpr const char *BIN_DIR = "../assets/shaders/bin/";

		static string glslUtils;
		static bool loaded;

		static void HandleGLSLValidateLog(const string &log);
		static void AddGLSLUtils(string &src);
		static void LoadGLSLUtils(void);
	};
}