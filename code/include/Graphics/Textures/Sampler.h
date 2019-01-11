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

	private:
		friend class DescriptorSet;

		SamplerHndl hndl;
		LogicalDevice &parent;

		void Destroy(void);
	};
}