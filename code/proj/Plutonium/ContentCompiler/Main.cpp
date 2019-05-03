#include "CompileToPum.h"
#include <Core/Diagnostics/Logging.h>

/*
Quickly checks if the command line argument matches.
a is the input which is case insensitive.
b is the match to check and must be upper case.
*/
bool clstrcmp(const char *a, const char *b)
{
	const size_t len = strlen(a);
	if (strlen(b) != len) return false;

	for (size_t i = 0; i < len; i++)
	{
		if (toupper(a[i]) != b[i]) return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	/*
	Check the command line arguments for the needed settings.
	Skip first as it's just the exe name.
	*/
	CLArgs args;
	for (size_t i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-o"))
		{
			if (i + 1 < argc) args.Output = argv[++i];
			else
			{
				Pu::Log::Error("Missing path for output file after -o!");
				args.IsValid = false;
			}
		}
		else if (!strcmp(argv[i], "-t"))
		{
			if (i + i < argc)
			{
				i++;
				if (clstrcmp(argv[i], "PUM")) args.Type = ContentType::PUM;
				else
				{
					Pu::Log::Error("'%s' is not a valid content type!", argv[i]);
					args.IsValid = false;
				}
			}
			else Pu::Log::Error("Missing type for input type after -t!");
		}
		else if (!strcmp(argv[i], "-n"))
		{
			if (i + 1 < argc) args.DisplayName = argv[++i];
			else Pu::Log::Error("Missing name for nput type after -n!");
		}
		else if (!strcmp(argv[i], "--help"))
		{
			Pu::Log::Message(
				"Usage: ContentCompiler [option]... [file]\n\n"
				"Options:\n"
				"-t\tSpecifies the required compile type.\n"
				"-o\tSpecifies the output file.");
			return EXIT_SUCCESS;
		}
		else if (i + 1 >= argc) args.Input = argv[i];
		else
		{
			Pu::Log::Error("'%s' is not a valid command line argument!\nUse --help for help.", argv[i]);
			args.IsValid = false;
		}
	}

	/* Make sure the input file was set. */
	if (!args.Input.length())
	{
		Pu::Log::Error("No input file was specified!");
		args.IsValid = false;
	}

	/* Try to generate the input file type. */
	if (args.Type == ContentType::Unknown)
	{
		const char *ext = strrchr(args.Input.c_str(), '.') + 1;

		if (clstrcmp(ext, "GLTF")) args.Type = ContentType::PUM;
		else if (clstrcmp(ext, "OBJ")) args.Type = ContentType::PUM;
		else if (clstrcmp(ext, "MD2")) args.Type = ContentType::PUM;
		else
		{
			Pu::Log::Error("File type cannot be deduced from file extension, specify -t for the type!");
			args.IsValid = false;
		}
	}

	/* Default the display name to the input file name without the extension. */
	if (!args.DisplayName.length()) args.DisplayName = args.Input.fileNameWithoutExtension();

	/* Default set the output file. */
	if (!args.Output.length())
	{
		const size_t len = args.Input.length() - strlen(strrchr(args.Input.c_str(), '.'));
		args.Output.resize(len, ' ');
		memcpy(args.Output.data(), args.Input.c_str(), len);
		
		switch (args.Type)
		{
		case ContentType::PUM:
			args.Output += ".pum";
			break;
		default:	// This will can only occur if previous functions failed so just skip.
			break;
		}
	}

	/* Exit here is the command line argument were invalid. */
	if (!args.IsValid) return EXIT_FAILURE;

	switch (args.Type)
	{
	case ContentType::PUM:
		return CompileToPum(args);
	default:
		Pu::Log::Error("Invalid type used in command line arguments!");
		return EXIT_FAILURE;
	}
}