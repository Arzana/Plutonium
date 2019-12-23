#pragma once
#include <tuple>
#include "Core/String.h"
#include "Streams/BinaryReader.h"
#include "Core/Events/DelegateObsevers.h"
#include "SPIRV.h"

namespace Pu
{
	/* Helper class to read specific SPIR-V modules. */
	class SPIRVReader
	{
	public:
		/* Defines a handler for reading a module. */
		using ModuleHandler = DelegateBase<SPIRVReader, spv::Op, size_t>;

		/* Creates a new reader from a raw source. */
		SPIRVReader(_In_ const void *src, _In_ size_t size);
		/* Creates a new reader from the specific file path. */
		SPIRVReader(_In_ const wstring &path);
		SPIRVReader(_In_ SPIRVReader&) = delete;
		SPIRVReader(_In_ SPIRVReader&&) = delete;
		/* Releases the resources allocated by the reader. */
		~SPIRVReader(void);

		_Check_return_ SPIRVReader& operator =(_In_ const SPIRVReader&) = delete;
		_Check_return_ SPIRVReader& operator =(_In_ SPIRVReader&&) = delete;

		/* Gets the SPIR-V version of the file. */
		_Check_return_ inline std::tuple<uint32, uint32> GetVersion(void) const
		{
			return std::make_tuple(major, minor);
		}

		/* Checks if another word can be read from the stream. */
		_Check_return_ inline bool CanReadWord(void) const
		{
			return reader->GetLocation() + sizeof(spv::Word) < reader->GetSize();
		}

		/* Read a single WORD from the stream. */
		_Check_return_ inline spv::Word ReadWord(void)
		{
			return reader->ReadUInt32();
		}

		/* Advances the stream by one word. */
		inline void AdvanceWord(void)
		{
			reader->Advance<spv::Word>();
		}

		/* Gets the pointer to the data SPIR-V data stream. */
		inline const BinaryReader& GetStream(void) const
		{
			return *reader;
		}

		/* Loops through all the modules defines and sends them to the handler. */
		void HandleAllModules(_In_ ModuleHandler& hndlr);
		/* Reads a null-terminated UTF-8 string from the stream. */
		_Check_return_ string ReadLiteralString(void);

	private:
		string raw;
		BinaryReader *reader;
		uint32 major, minor;
		size_t bound;

		void ValidateHeader(void);
		std::tuple<spv::Op, size_t> ReadInstructionHeader(void);
	};
}