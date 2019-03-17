#include "Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h"
#include "Streams/FileReader.h"
#include "Streams/FileWriter.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Threading/ThreadUtils.h"
#include "Graphics/Vulkan/VulkanGlobals.h"
#include "Config.h"

using namespace Pu;

bool Pu::SPIRV::loaded = false;
string Pu::SPIRV::glslUtils = "";

wstring Pu::SPIRV::FromGLSLPath(const wstring & path)
{
	/* Make sure the directory is created. */
	FileWriter::CreateDirectory(BIN_DIR);
	const wstring curDir = FileReader::GetCurrentDirectory() + L'\\';

	/*
	Create arguments for the validator.
	-V: Indicates that a SPIR-V binary shuold be created with the latest version.
	-H: Specifies that the validator should print a human readable version of the result.
	-o: Specifies the output path.
	*/
	const wstring fname = path.fileName();
	const wstring input = curDir + path;
	const wstring output = curDir + BIN_DIR + fname + L".spv";

	wstring args = (SpirVCompilerLogHumanReadable ? L"-V -H -o \"" : L"-V -o \"") + output + L"\" \"" + input + L'\"';
	wstring log;

	/* Run the validator. */
	const bool succeeded = _CrtRunProcess(L"glslangValidator.exe", args, log, SpirVCompilerTimeout);
	HandleGLSLValidateLog(log, input);

	/* Log either success or failure. */
	if (succeeded)
	{
		Log::Verbose("Compiled GLSL source '%ls' to SPIR-V.", fname.c_str());
		return output;
	}
	else
	{
		Log::Error("Unable to compile GLSL source '%ls' to SPIR-V (Could not run validator)!", fname.c_str());
		return L"";
	}
}

void Pu::SPIRV::HandleGLSLValidateLog(const wstring & log, const wstring & path)
{
	/* Split the log into it's lines and remove empty lines. */
	vector<wstring> lines = log.split(L"\r\n");
	lines.tryRemove(L"\n");

	/*
	Only log if something has been logged.
	There are only two one line messages:
	- File cannot be opened (we search for a compiled version later so that will crash if this is the case).
	- Echo back file path (useless message).
	*/
	if (lines.size() > 1)
	{
		const wstring seperator = wstring(64, L'-') + L'\n';

		/* Log header. */
		wstring msg = L"glslangValidator log for '" + path + L":\n";
		msg += seperator;

		for (wstring &line : lines)
		{
			/* Remove the path as it's appended to all lines also remove any leading newlines. */
			line.remove(L'\n');
			line.remove(path);

			/* Only log if useful information is left in the line. */
			if (line.length() > 1) msg += line += L'\n';
		}

		/* Log footer. */
		msg += seperator;
		Log::Verbose("%ls", msg.c_str());
	}
}

void Pu::SPIRV::AddGLSLUtils(string & src)
{
	/* Try to load the utilities and add them to the start of the source. */
	LoadGLSLUtils();
	src.insert(0, SPIRV::glslUtils);
}

void Pu::SPIRV::LoadGLSLUtils(void)
{
	constexpr const wchar_t *PATH = L"../include/Graphics/Vulkan/SPIR-V/Utilities.glsl";

	/* Early out if already loaded. */
	if (loaded) return;
	loaded = true;

	/* Attempt to open utilities file. */
	FileReader fr(PATH);
	if (!fr.IsOpen())
	{
		Log::Warning("Unable to load Plutonium GLSL utilities, compiling code without utility functions!");
		return;
	}

	/* Copy the contents of the file to our buffer. */
	SPIRV::glslUtils = std::move(fr.ReadToEnd());
}