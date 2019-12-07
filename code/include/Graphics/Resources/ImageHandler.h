#pragma once
#include "Core/String.h"
#include "Core/Threading/Tasks/Task.h"
#include "Graphics/Vulkan/VulkanEnums.h"

namespace Pu
{
	/* Defines the information given by the image loader and needed by the image saver. */
	struct ImageInformation
	{
	public:
		/* The width of the image. */
		uint32 Width;
		/* The hight of the image.s */
		uint32 Height;
		/* The amount of bytes per fragment. */
		uint32 Components;
		/* Whether the image is HDR. */
		bool IsHDR;

		/* Initializes an empty instance of the image information object. */
		ImageInformation(void)
			: Width(3), Height(3), Components(4), IsHDR(false)
		{}

		/* Initializes a new instance of the image information object. */
		ImageInformation(_In_ int32 w, _In_ int32 h, _In_ int32 c, _In_ int hdr)
			: Width(static_cast<uint32>(w)), Height(static_cast<uint32>(h)), Components(static_cast<uint32>(c)), IsHDR(hdr)
		{}

		/* Gets the Vulkan format for the image. */
		_Check_return_ Format GetImageFormat(_In_ bool sRGB) const;
	};

	/* Gets the extent of an image. */
	_Check_return_ ImageInformation _CrtGetImageInfo(_In_ const wstring &path);
	/* Loads an image from the specified file as a HDR image with the desired amount of channels. */
	_Check_return_ vector<float> _CrtLoadImageHDR(_In_ const wstring &path);
	/* Loads an image from the specified file as a LDR image with the desired amount of channels. */
	_Check_return_ vector<byte> _CrtLoadImageLDR(_In_ const wstring &path);
	/* Load an image from the specified file as a LDR image and gets the information about the image. */
	_Check_return_ vector<byte> _CrtLoadImageLDR(_In_ const wstring &path, _Out_ ImageInformation &info);

	/* Defines a way to load a image from file. */
	template <typename component_t>
	class ImageLoadTask
		: public Task
	{
	public:
		/* Initializes a new instance of an image load task. */
		ImageLoadTask(_In_ const wstring &path)
			: path(path)
		{}

		ImageLoadTask(_In_ const ImageLoadTask&) = delete;

		_Check_return_ ImageLoadTask& operator =(_In_ const ImageLoadTask&) = delete;

		/* Loads the image content. */
		_Check_return_ virtual Result Execute(void) override
		{
			if constexpr (std::is_same<component_t, float>::value)
			{
				result = _CrtLoadImageHDR(path);
			}
			else if constexpr (std::is_same<component_t, byte>::value)
			{
				result = _CrtLoadImageLDR(path);
			}
			else static_assert(true, "Invalid component type!");

			return Result::Default();
		}

		/* Gets the data loaded by the task. */
		_Check_return_ inline const vector<component_t>& GetData(void) const
		{
			return result;
		}

	private:
		const wstring path;
		vector<component_t> result;
	};
}