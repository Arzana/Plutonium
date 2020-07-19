#pragma once
#include "Graphics/Resources/DynamicBuffer.h"
#include "Graphics/VertexLayouts/PointLight.h"

namespace Pu
{
	/* Defines a instance pool used to render multiple point lights at once. */
	class PointLightPool
		: public DynamicBuffer
	{
	public:
		/* Initializes a new instance of a point light pool with a specific maximum amount of lights. */
		PointLightPool(_In_ LogicalDevice &device, _In_ uint32 maxLights);
		PointLightPool(_In_ const PointLightPool&) = delete;
		/* Move constructor. */
		PointLightPool(_In_ PointLightPool &&value) = default;

		_Check_return_ PointLightPool& operator =(_In_ const PointLightPool&) = delete;
		/* Move assignment. */
		_Check_return_ PointLightPool& operator =(_In_ PointLightPool &&other) = default;

		/* Gets whether this pool has space for an additional light. */
		_Check_return_ inline bool HasSpace(void) const
		{
			return buffer.capacity() > buffer.size();
		}

		/* Gets the amount of lights currently in this pool. */
		_Check_return_ inline uint32 GetLightCount(void) const
		{
			return static_cast<uint32>(buffer.size());
		}

		/* Adds a new light to the pool. */
		void AddLight(_In_ Vector3 position, _In_ Color color, _In_ float attenuationConstant, _In_ float attenuationLinear, _In_ float attenuationQuadratic);

	private:
		vector<PointLight> buffer;

		void StageLightBuffer(void);
	};
}