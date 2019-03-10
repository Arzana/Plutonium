#pragma once
#include "Graphics/Vulkan/VulkanObjects.h"
#include "Field.h"

/* Redefine the offsetof macro so we can easily use it with our attribute. */
#define vkoffsetof(container, member)	static_cast<uint32>(offsetof(container, member))

namespace Pu
{
	/* Specifies information about a shaders vertex attributes. */
	class Attribute
		: public Field 
	{
	public:
		/* Overrides the default format given to the attribute. */
		inline void SetFormat(_In_ Format format)
		{
			description.Format = format;
		}

		/* Sets the binding index. */
		inline void SetBinding(_In_ uint32 binding)
		{
			description.Binding = binding;
		}

		/* Sets the offset of the attribute in the buffer layout. */
		inline void SetOffset(_In_ uint32 offset)
		{
			description.Offset = offset;
		}

	private:
		friend class Renderpass;
		friend class GraphicsPipeline;

		VertexInputAttributeDescription description;

		Attribute(const FieldInfo &data);
	};
}