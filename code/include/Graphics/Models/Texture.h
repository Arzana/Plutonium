#pragma once
#include "Core\Math\Constants.h"
#include "Core\Math\Rectangle.h"

namespace Plutonium
{
	/* Defines a basic 2D texture used in rendering. */
	struct Texture
	{
		/* The width of the texture. */
		const int32 Width;
		/* The height of the texture. */
		const int32 Height;
		/* The levels of mipmapping used in this texture. */
		const int32 MipMapLevels;

		/* Initializes a new instance of an empty 2D texture. */
		Texture(_In_ int32 width, _In_ int32 height, _In_opt_ int32 mipMapLevels = 4, _In_opt_ const char *name = nullptr);
		Texture(_In_ const Texture &value) = delete;
		Texture(_In_ Texture &&value) = delete;
		/* Releases the resources allocated for the texture. */
		~Texture(void);

		_Check_return_ Texture& operator =(_In_ const Texture &other) = delete;
		_Check_return_ Texture& operator =(_In_ Texture &&other) = delete;

		/* Loads a specified texture from a file (requires delete!). */
		_Check_return_ static Texture* FromFile(_In_ const char *path);

		/* Gets the name assigned to the texture. */
		_Check_return_ inline const char* GetName(void) const
		{
			return name;
		}

		/* Gets the size of the texture as a vector. */
		_Check_return_ inline Vector2 GetSize(void) const
		{
			return Vector2(static_cast<float>(Width), static_cast<float>(Height));
		}

		/* Gets the full bounds of the texture. */
		_Check_return_ inline Rectangle GetBounds(void) const
		{
			return Rectangle(GetSize());
		}

		/* Get the amount of channels stored in the texture. */
		_Check_return_ int32 GetChannels(void) const;
		/* Sets the raw data of the texture to the specified data (data is expected to be Width*Height in size!). */
		void SetData(_In_ byte *data);
		/* Gets a copy of the data specified for the texture (requires free!). */
		_Check_return_ byte* GetData(void) const;
		/* Saves the tetxure as a specified file. */
		void SaveAsPng(_In_ const char *path);

	protected:
		uint32 ptr;
		const char *name;

	private:
		friend struct Uniform;

		int32 frmt, ifrmt;

		static int32 GetMaxMipMapLevel(int32 w, int32 h);

		void Dispose(void);
		void SetFormat(uint32 channels);
		void GenerateTexture(const void *data);
	};
}