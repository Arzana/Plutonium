#include "CompileToPum.h"
#include <Core/Diagnostics/Logging.h>

using namespace Pu;

void logHelp(void)
{
	Log::Message(
		"Usage: ContentCompiler [option]... [file]\n\n"
		"Options:\n"
		"--help				Displays this message.\n"
		"-o <path>			Specifies the output file.\n"
		"-dn <name>			Overrides the default model name.\n"
		"-n					(Re)calculate face normals.\n"
		"-t					(Re)calculate vertex tangents.\n"
		"-rf				Reorders the verices in the faces to match backface culling.\n"
		"-at <path>;<path>;	Adds the specified textures to the output model.");
}

int initCmdLineArgs(const vector<string> &args, CLArgs &result)
{
	int state = EXIT_SUCCESS;

	for (size_t i = 0; i < args.size(); i++)
	{
		const string &cur = args[i];
		const bool notLast = i + 1 < args.size();

		if (cur == "--help")		// Help output.
		{
			logHelp();
			return EXIT_SUCCESS;
		}
		else if (cur == "-o")		// Output path.
		{
			if (notLast) result.Output = args[++i];
			else
			{
				Log::Error("Missing path for output file after -o!");
				state = EXIT_FAILURE;
			}
		}
		else if (cur == "-dn")		// Display name.
		{
			if (notLast) result.Output = args[++i];
			else
			{
				Log::Error("Missing name for output file after -dn!");
				state = EXIT_FAILURE;
			}
		}
		else if (cur == "-n")		// Generate normals.
		{
			if (!result.ReorderFaces) result.RecalcNormals = true;
			else
			{
				Log::Error("Recalculate normals (-n) cannot be active at the same time as reoder face vertices (-rf)!");
				state = EXIT_FAILURE;
			}
		}
		else if (cur == "-t")		// Generate tangents.
		{
			result.RecalcTangents = true;
		}
		else if (cur == "-at")		// Additional textures.
		{
			if (notLast) result.AdditionalTextures = args[++i].split(';');
			else
			{
				Log::Error("Missing textures for additional textures option!");
				state = EXIT_FAILURE;
			}
		}
		else if (cur == "-rf")		// Reorder face vertices.
		{
			if (!result.RecalcNormals) result.ReorderFaces = true;
			else
			{
				Log::Error("Reoder face vertices (-rf) cannot be active at the same time as recalculate normales (-n)!");
				state = EXIT_FAILURE;
			}
		}
		else if (!notLast)			// Input file (must be the last else if statement!).
		{
			result.Input = cur;
		}
		else
		{
			Log::Error("'%s' is not recognized as a valid command line argument!", cur.c_str());
			state = EXIT_FAILURE;
		}
	}

	return state;
}

int setDefaultArgs(CLArgs &args)
{
	/* Set the output type to the correct value. */
	const string ext = args.Input.fileExtension().toUpper();
	if (ext == "GLTF" || ext == "GLB" || ext == "OBJ" || ext == "MD2") args.Type = ContentType::PUM;
	else
	{
		Log::Error("Cannot deduce output type from input file type '%s'!", ext.c_str());
		return EXIT_FAILURE;
	}

	/* Set the default display name if none is specified. */
	if (args.DisplayName.empty()) args.DisplayName = args.Input.fileNameWithoutExtension();

	/* Set the default output file if none is specified. */
	if (args.Output.empty())
	{
		args.Output = args.Input.fileWithoutExtension();
		if (args.Type == ContentType::PUM) args.Output += ".pum";
	}

	return EXIT_SUCCESS;
}

int run(const vector<string> &args)
{
	CLArgs finalArgs;

	/* Check the user input. */
	if (initCmdLineArgs(args, finalArgs) == EXIT_FAILURE) return EXIT_FAILURE;
	if (setDefaultArgs(finalArgs) == EXIT_FAILURE) return EXIT_FAILURE;

	/* Final command line check. */
	if (finalArgs.Input.empty())
	{
		Log::Error("No input file was specified!");
		return EXIT_FAILURE;
	}

	/* Set the user parameter to the file name, so we can use it in MSBuild. */
	Log::SetUserInfo(finalArgs.Input.fileName());

	if (finalArgs.Type == ContentType::PUM) return CompileToPum(finalArgs);
	else
	{
		Log::Fatal("Unknown type was set, this should never occur!");
		return EXIT_FAILURE;
	}
}

int main(int argc, char **argv)
{
	try
	{
		/* We only show the type, this is easy to put into a MSBuild error regex. */
		Log::SetDetails(LogDetails::Type);

		vector<string> args;
		args.reserve(argc);

		/* Skip the exe identity. */
		for (int i = 1; i < argc; i++) args.emplace_back(argv[i]);
		return run(args);
	}
	catch (...)
	{
		/* This is to make sure that we don't crash on content compiling but simply return a failed state. */
		return EXIT_FAILURE;
	}
}