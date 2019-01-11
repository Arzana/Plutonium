#pragma once
#include "Sampler.h"
#include "Graphics/Vulkan/ImageView.h"

namespace Pu
{
	/* Defines a base object for all texture types. */
	class Texture
		: public Image
	{
	public:
		Texture(_In_ const Texture&) = delete;
		/* Move assignment. */
		Texture(_In_ Texture &&value);
		/* Destroys the texture. */
		virtual ~Texture(void)
		{
			Destroy();
		}

		_Check_return_ Texture& operator =(_In_ const Texture&) = delete;
		/* Move assignment. */
		_Check_return_ Texture& operator =(_In_ Texture &&other);

	protected:
		Texture(_In_ LogicalDevice &device, _In_ Sampler &sampler, _In_ const ImageCreateInfo &createInfo);

	private:
		friend class DescriptorSet;

		Sampler &sampler;
		ImageView *view;

		void Destroy(void);
	};
}