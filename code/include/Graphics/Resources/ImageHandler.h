#pragma once
#include "Core/String.h"
#include "Core/Threading/Tasks/Task.h"

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

	/* Defines a way to load a image from file. */
	class ImageLoadTask
		: public Task
	{
	public:
		/* Initializes a new instance of an image load task. */
		ImageLoadTask(_In_ const string &path, _In_opt_ int32 desiredChannels = 0);
		ImageLoadTask(_In_ const ImageLoadTask&) = delete;

		_Check_return_ ImageLoadTask& operator =(_In_ const ImageLoadTask&) = delete;

		/* Loads the image content. */
		_Check_return_ virtual Result Execute(void) override;

		/* Gets the data loaded by the task. */
		_Check_return_ inline const vector<float>& GetData(void) const 
		{
			return result;
		}

		/* Gets the information of the loaded image. */
		_Check_return_ inline const ImageInformation& GetInfo(void) const 
		{
			return info;
		}

	private:
		const string path;
		vector<float> result;
		ImageInformation info;
		int32 channels;
	};

	/* Loads an image from the specified file as a HDR image with the desired amount of channels. */
	_Check_return_ vector<float> _CrtLoadImage(_In_ const string &path, _Out_ ImageInformation &info, _In_opt_ int32 desiredChannels = 0);
}