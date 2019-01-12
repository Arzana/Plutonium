#pragma once
#include "Graphics/Vulkan/LogicalDevice.h"

namespace Pu
{
	/* Defines an object that defines how a resource is sampled. */
	class Sampler
	{
	public:
		/* Initializes a new instance of a sampler. */
		Sampler(_In_ LogicalDevice &device, _In_ const SamplerCreateInfo &createInfo);
		Sampler(_In_ const Sampler&) = delete;
		/* Move constructor. */
		Sampler(_In_ Sampler &&value);
		/* Destroys the sampler. */
		~Sampler(void)
		{
			Destroy();
		}

		_Check_return_ Sampler& operator =(_In_ const Sampler&) = delete;
		/* Move assignment. */
		_Check_return_ Sampler& operator =(_In_ Sampler &&other);
		/* Gets whether the specified sampler is functionally equal to this sampler. */
		_Check_return_ bool operator ==(_In_ const Sampler &other) const;
		/* Gets whether the specified sampler is functionally different to this sampler. */
		_Check_return_ inline bool operator !=(_In_ const Sampler &other) const
		{
			return !operator==(other);
		}

	private:
		friend class DescriptorSet;

		SamplerHndl hndl;
		LogicalDevice &parent;

		Filter magFilter, minFilter;
		SamplerMipmapMode mipmapMode;
		SamplerAddressMode uMode, vMode, wMode;
		float loDBias, maxAnisotropy, minLoD, maxLoD;
		bool anisotropy, compare, unnormalizedCoordinates;
		CompareOp cmpOp;
		BorderColor clr;

		void Destroy(void);
	};
}