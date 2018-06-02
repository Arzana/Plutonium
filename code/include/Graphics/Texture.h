#pragma once
#include "Core\Math\Constants.h"
#include "Core\Math\Rectangle.h"
#include "Graphics\Color.h"
#include "Graphics\TextureOptions.h"
#include "Graphics\Native\Window.h"

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
		Texture(_In_ int32 width, _In_ int32 height, _In_ WindowHandler wnd, _In_opt_ const TextureCreationOptions *config = &TextureCreationOptions::Default2D, _In_opt_ const char *name = nullptr);
		Texture(_In_ const Texture &value) = delete;
		Texture(_In_ Texture &&value) = delete;
		/* Releases the resources allocated for the texture. */
		~Texture(void);

		_Check_return_ Texture& operator =(_In_ const Texture &other) = delete;
		_Check_return_ Texture& operator =(_In_ Texture &&other) = delete;

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
		/* Saves the tetxure as a specified file. */
		void SaveAsPng(_In_ const char *path);

		/* Gets a copy of the underlying data specified by the texture (byte, float and Color are accepted types). */
		template <typename _Ty>
		_Check_return_ void GetData(_Out_ _Ty *buffer) const;

	private:
		friend struct Uniform;
		friend struct AssetLoader;
		friend struct RenderTarget;

		static constexpr size_t CUBEMAP_TEXTURE_COUNT = 6;

		uint32 ptr;
		const TextureCreationOptions config;
		const char *path;
		const char *name;
		int32 frmt, ifrmt, type;
		WindowHandler wnd;

		static Texture* FromFile(const char *path, WindowHandler wnd, const TextureCreationOptions *config = &TextureCreationOptions::Default2D);
		static Texture* FromFile(const char *paths[CUBEMAP_TEXTURE_COUNT], WindowHandler wnd, const TextureCreationOptions *config = &TextureCreationOptions::DefaultCube);
		static int32 GetMaxMipMapLevel(int32 w, int32 h);

		void Dispose(void);
		bool SetFormat(uint32 channels);
		void ConvertFormat(byte **data, int32 srcChannels, int32 destChannels);
		void GenerateTexture(byte **data);
		void SetPreDataTransferTextureOptions(byte *data);
		void SetPostDataTransferTextureOptions(void);

		void GetDataAsBytes(byte *buffer) const;
		void GetDataAsFloats(float *buffer) const;
		void GetDataAsRGB(Color *buffer) const;
		void GetDataAsRGBA(Color *buffer) const;
	};

	template<typename _Ty>
	inline void Texture::GetData(_Ty * buffer) const
	{
		static_assert(false, "Invalid buffer type specified!");
	}

	template <>
	inline void Texture::GetData<byte>(byte * buffer) const
	{
		ASSERT_IF(config.IsDepth, "Data from depth maps can only be requested as floats!");
		GetDataAsBytes(buffer);
	}

	template <>
	inline void Texture::GetData<float>(float * buffer) const
	{
		ASSERT_IF(!config.IsDepth, "Cannot request data from color map as floats!");
		GetDataAsFloats(buffer);
	}

	template <>
	inline void Texture::GetData<Color>(Color * buffer) const
	{
		ASSERT_IF(config.IsDepth, "Data from depth maps can only be requested as floats!");
		ASSERT_IF(frmt == GL_RED, "Cannot request data from greyscale map as colors!");

		if (frmt == GL_RGB) GetDataAsRGB(buffer);
		else if (frmt == GL_RGBA) GetDataAsRGBA(buffer);
		else ASSERT("Invalid internal format specified, this should never occur!");
	}
}