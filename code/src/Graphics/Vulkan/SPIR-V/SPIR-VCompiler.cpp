#include "Graphics/Vulkan/SPIR-V/SPIR-VCompiler.h"
#include "Streams/FileReader.h"
#include "Streams/FileWriter.h"
#include "Core/Diagnostics/Logging.h"
#include "Streams/FileUtils.h"
#include "Core/Threading/ThreadUtils.h"
#include "Graphics/Vulkan/VulkanGlobals.h"

using namespace Pu;

bool Pu::SPIRV::loaded = false;
string Pu::SPIRV::glslUtils = "";

string Pu::SPIRV::FromGLSL(const string & src)
{
	return "";
}

string Pu::SPIRV::FromGLSLPath(const string & path)
{
	/* Make sure the directory is created. */
	FileWriter::CreateDirectory(BIN_DIR);
	const string curDir = FileReader::GetCurrentDirectory() + '\\';

	/* Create arguments for the validator. */
	const string fname = _CrtGetFileName(path);
	const string output = curDir + BIN_DIR + fname + ".spv";
	const string args = "-V -H -o \"" + output + "\" \"" + curDir + path + '\"';
	string log;

	/* Run the validator. */
	const bool succeeded = _CrtRunProcess("glslangValidator.exe", const_cast<char*>(args.c_str()), log);
	HandleGLSLValidateLog(log);

	/* Log either success or failure. */
	if (succeeded) Log::Verbose("Compiled GLSL source '%s' to SPIR-V.", fname.c_str());
	else Log::Error("Unable to compile GLSL source to SPIR-V (Could not run validator)!");

	return "";
}

void Pu::SPIRV::HandleGLSLValidateLog(const string & log)
{
	/* Split the log into it's lines. */
	vector<string> lines = log.split("\r\n");

	/* Only log if something has been logged. */
	if (lines.size() > 0)
	{
		/* Log header. */
		Log::Verbose("glslangValidator log:");
		Log::Verbose(string(64, '-').c_str());

		for (string &line : lines)
		{
			/* Remove leading newlines. */
			line.remove('\n');
			if (line.length() > 0)
			{
				/* If the message has 'ERROR: ' in it log it as an error, otherwise just verbose. */
				const size_t off = line.find("ERROR: ");
				if (off != string::npos) Log::Error(line.c_str() + off + 7);
				else Log::Verbose(line.c_str());
			}
		}

		/* Log footer. */
		Log::Verbose(string(64, '-').c_str());
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
	constexpr const char *PATH = "../include/Graphics/Vulkan/SPIR-V/Utilities.glsl";

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