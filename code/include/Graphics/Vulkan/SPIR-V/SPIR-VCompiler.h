#pragma once
#include "Core/String.h"

namespace Pu
{
	/* Helper object to convert from high level languages to SPIR-V. */
	class SPIRV
	{
	public:
		/* Converts from a GLSL code file to SPIR-V source. */
		_Check_return_ static wstring FromGLSLPath(_In_ const wstring &path);

	private:
		static constexpr const wchar_t *BIN_DIR = L"../assets/shaders/bin/";

		static string glslUtils;
		static bool loaded;

		static void HandleGLSLValidateLog(const wstring &log, const wstring &path);
		static void AddGLSLUtils(string &src);
		static void LoadGLSLUtils(void);
	};
}