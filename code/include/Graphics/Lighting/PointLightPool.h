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

		/* Clears the host buffer of point lights. */
		inline void Clear(void)
		{
			buffer.clear();
		}

		/* Calculates the radius of the point light volume based on the specified parameters and the cutoff point. */
		_Check_return_ static float GetLightRadius(_In_ Vector3 color, _In_ float intensity, _In_ float falloffLinaer, _In_ float falloffQuadratic, _In_opt_ uint8 cutoff = 5);
		/* Adds a new light to the pool. */
		void AddLight(_In_ Vector3 position, _In_ Color color, _In_ float intensity, _In_ float falloffLinear, _In_ float falloffQuadratic);
		/* Updates the point light pool if needed. */
		virtual void Update(_In_ CommandBuffer &cmdBuffer) override;
		/* Forces the host light buffer to update the GPU buffer. */
		void StageLightBuffer(void);

	private:
		bool isDirty;
		vector<PointLight> buffer;
	};
}