#pragma once
#include "Texture.h"

namespace Pu
{
	/* Defines a 1x1 texture of a single colors that can be used as a deafult texture. */
	class DefaultTexture
		: public Texture
	{
	public:
		/* Initializes a new instance of a default texture for the specified solid color. */
		DefaultTexture(_In_ LogicalDevice &device, _In_ Color color);
		DefaultTexture(_In_ const Texture&) = delete;
		/* Move constructor. */
		DefaultTexture(_In_ DefaultTexture &&value);
		/* Destroys the default texture. */
		virtual ~DefaultTexture(void)
		{
			Destroy();
		}

		_Check_return_ DefaultTexture& operator =(_In_ const DefaultTexture&) = delete;
		/* Move assignment. */
		_Check_return_ DefaultTexture& operator =(_In_ DefaultTexture &&other);

		/* Stages the contents of the default image (needs to be pushed to a transfer compatible queue). */
		void Initialize(_In_ CommandBuffer &cmdBuffer);
		/* Deletes the staging resources, this action makes the texture ready for use (needs to be pushed to a graphics compatible queue). */
		void Finalize(_In_ CommandBuffer &cmdBuffer);
		/* Stages the contents of the default image and makes the texture ready for use (needs to be pushed to a transfer/graphics compatible queue). */
		void Create(_In_ CommandBuffer &cmdBuffer);

		/* Gets the color of this default texture. */
		_Check_return_ inline Color GetColor(void) const
		{
			return color;
		}

	private:
		Pu::Sampler *resSampler;
		Pu::Image *resImage;

		StagingBuffer *buffer;
		Color color;

		Pu::Sampler& CreateAndSetSampler(LogicalDevice &device);
		Pu::Image& CreateAndSetImage(LogicalDevice &device);
		void MarkImage(void);
		void Destroy(void);
	};
}