#pragma once
#include "Core/String.h"

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

		/* Initializes an empty instance of the image information object. */
		ImageInformation(void)
			: Width(0), Height(0), Components(0)
		{}
	};

	/* Loads an image from the specified file as a HDR image with the desired amount of channels. */
	_Check_return_ vector<float> _CrtLoadImage(_In_ const string &path, _Out_ ImageInformation &info, _In_opt_ int32 desiredChannels = 0);
}