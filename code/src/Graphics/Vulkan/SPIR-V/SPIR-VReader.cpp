#include "Graphics/Vulkan/SPIR-V/SPIR-VReader.h"
#include "Streams/FileReader.h"
#include "Core/Diagnostics/Logging.h"

Pu::SPIRVReader::SPIRVReader(const wstring & path)
{
	/* Load raw binary data from file and initialize the binary reader. */
	raw = FileReader(path).ReadToEnd();
	reader = new BinaryReader(raw.data(), raw.size());

	/* Validate the header. */
	ValidateHeader();
}

Pu::SPIRVReader::~SPIRVReader(void)
{
	delete reader;
}

void Pu::SPIRVReader::HandleAllModules(ModuleHandler hndlr)
{
	/* Loop untill the end of the file has been reached. */
	while (CanReadWord())
	{
		/* Read the module header and store the read location. */
		auto[opCode, wordCnt] = ReadInstructionHeader();
		const size_t start = reader->GetLocation();

		/* Let user handle the module. */
		hndlr.HandlePost(*this, opCode, wordCnt);

		/* Advance all bytes that the user hasn't handled. */
		const size_t advanced = reader->GetLocation() - start;
		const size_t wordsAdvanced = advanced / sizeof(spv::Word);
		if (wordsAdvanced > wordCnt) Log::Fatal("Module handler has read outside of module space!");
		if (wordsAdvanced < wordCnt) reader->Seek(SeekOrigin::Current, static_cast<int64>(wordCnt * sizeof(spv::Word) - advanced));
	}
}

Pu::string Pu::SPIRVReader::ReadLiteralString(void)
{
	string str;

	/* Read untill null-terminator is found. */
	spv::Word word;
	while ((word = ReadWord()) != 0)
	{
		/* String is stored as 4 chars per word (little endian). */
		for (size_t i = 0; i < sizeof(spv::Word) * 8; i += 8)
		{
			const octet byte = (word & 0xFF << i) >> i;
			if (!byte) goto End;

			/* Get current octet from word. */
			str += static_cast<char>(byte);
		}
	}

	End:
	return str;
}

void Pu::SPIRVReader::ValidateHeader(void)
{
	/* Check if the file is an actual SPIR-V file. */
	if (ReadWord() != spv::MagicNumber) Log::Fatal("Attempting to parse non SPIR-V file!");

	/* Read the version (lower order to higher order). */
	reader->Advance<byte>();
	minor = reader->ReadByte();
	major = reader->ReadByte();
	reader->Advance<byte>();

	/* Skip the generators version, read the ID bound and skip the zero WORD. */
	AdvanceWord();
	bound = ReadWord();
	AdvanceWord();
}

std::tuple<spv::Op, size_t> Pu::SPIRVReader::ReadInstructionHeader(void)
{
	/* Returns a module's operation code and the remaining byte count of the module. */
	const spv::Word word = ReadWord();
	return std::make_tuple(_CrtInt2Enum<spv::Op>(word & spv::OpCodeMask), (word >> spv::WordCountShift) - 1);
}