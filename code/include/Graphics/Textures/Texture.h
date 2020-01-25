#pragma once
#include "Sampler.h"
#include "Core/Threading/Tasks/Task.h"
#include "Graphics/Vulkan/ImageView.h"
#include "Graphics/Resources/ImageHandler.h"
#include "Graphics/Resources/StagingBuffer.h"

namespace Pu
{
	/* Defines a base object for all texture types. */
	class Texture
	{
	public:
		Texture(_In_ const Texture&) = delete;
		/* Move constructor. */
		Texture(_In_ Texture &&value);
		/* Destroys the texture. */
		virtual ~Texture(void)
		{
			Destroy();
		}

		_Check_return_ Texture& operator =(_In_ const Texture&) = delete;
		/* Move assignment. */
		_Check_return_ Texture& operator =(_In_ Texture &&other);

		/* Implicit convertion to get the image. */
		_Check_return_ inline operator const Image&(void) const
		{
			return *Image;
		}

		/* Gets whether the underlying image is usable. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return Image->IsLoaded();
		}

		/* Gets the underlying image. */
		_Check_return_ inline Image& GetImage(void)
		{
			return *Image;
		}

		/* Gets the underlying image. */
		_Check_return_ inline const Image& GetImage(void) const
		{
			return *Image;
		}

		/* Gets a view to the underlying image. */
		_Check_return_ inline const ImageView& GetView(void) const
		{
			return *view;
		}

		/* Gets a sub-resource range spaning all sub-resources. */
		_Check_return_ inline ImageSubresourceRange GetFullRange(void) const 
		{
			return Image->GetFullRange(ImageAspectFlag::Color);
		}

		/* Gets the extent of the image. */
		_Check_return_ inline Extent3D GetExtent(void) const
		{
			return Image->GetExtent();
		}

	protected:
		/* The sampler used to sample the texure. */
		Sampler *Sampler;
		/* The image data of the texture. */
		Image *Image;

		Texture(_In_ Pu::Sampler &sampler, _In_ Pu::Image &image, _In_ ImageViewType type);

	private:
		friend class DescriptorSet;
		friend class AssetLoader;
		friend class AssetFetcher;

		class LoadTask
			: public Task
		{
		public:
			LoadTask(Texture &result, const ImageInformation &info, const wstring &path);
			LoadTask(const LoadTask&) = delete;
			~LoadTask(void);

			LoadTask& operator =(const LoadTask&) = delete;

			virtual Result Execute(void) override;
			virtual Result Continue(void) override;

			inline StagingBuffer& GetStagingBuffer(void)
			{
				return *stagingBuffer;
			}

		private:
			Texture &result;
			ImageInformation info;
			Task *child;
			StagingBuffer *stagingBuffer;
			wstring path;
		};

		ImageView *view;

		void Destroy(void);
	};
}