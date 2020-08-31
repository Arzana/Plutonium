#pragma once
#include "Core/Math/Matrix.h"
#include "Graphics/Textures/Texture2D.h"
#include "Graphics/Vulkan/DescriptorSet.h"
#include "Graphics/Textures/Texture2DArray.h"

namespace Pu
{
	/* Defines a terrain material descriptor. */
	class Terrain
		: public DescriptorSet
	{
	public:
		/* Initializes a new instance of a terrain descriptor set. */
		Terrain(_In_ DescriptorPool &pool, _In_ const DescriptorSetLayout &layout);
		Terrain(_In_ const Terrain&) = delete;
		/* Move constructor. */
		Terrain(_In_ Terrain &&value) = default;

		_Check_return_ Terrain& operator =(_In_ const Terrain&) = delete;
		/* Move assignment. */
		_Check_return_ Terrain& operator =(_In_ Terrain &&other) = default;

		/* Gets the transformation matrix of this terain. */
		_Check_return_ const Matrix& GetTransform(void) const;
		/* Sets the location of the terrain. */
		void SetPosition(_In_ Vector3 value);
		/* Sets the scale of the terrain. */
		void SetScale(_In_ float value);

		/* Sets the displacement factor for the terrain. */
		inline void SetDisplacement(_In_ float value)
		{
			factors.X = value;
		}

		/* Sets the tessellation factor for the terrain. */
		inline void SetTessellation(_In_ float value)
		{
			factors.Y = value;
		}

		/* Sets the tessellation edge factor for the terrain. */
		inline void SetEdgeSize(_In_ float value)
		{
			factors.Z = value;
		}

		/* Sets the width/height of a single terrain patch. */
		inline void SetPatchSize(_In_ float value)
		{
			factors.W = value;
		}

		/* Sets the height map for this terrain. */
		inline void SetHeight(_In_ const Texture2D &value)
		{
			Write(*height, value);
		}

		/* Sets the texture mask map for this terrain. */
		inline void SetMask(_In_ const Texture2D &value)
		{
			Write(*mask, value);
		}

		/* Sets the diffuse textures for this terrain. */
		inline void SetTextures(_In_ const Texture2DArray &value)
		{
			Write(*textures, value);
		}

		/* Gets the displacement factor of this terrain. */
		_Check_return_ inline float GetDisplacement(void) const
		{
			return factors.X;
		}

		/* Gets the tessellation factor of this terrain. */
		_Check_return_ inline float GetTessellation(void) const
		{
			return factors.Y;
		}

		/* Gets the tessellation edge factor of this terrain. */
		_Check_return_ inline float GetEdgeSize(void) const
		{
			return factors.Z;
		}

		/* Gets the patch size of this terrain. */
		_Check_return_ inline float GetPatchSize(void) const
		{
			return factors.W;
		}

		/* Gets the scale of this terrain. */
		_Check_return_ inline float GetScale(void) const
		{
			return transform.W;
		}

	protected:
		/* Stages the buffer data for the uniform buffer. */
		void Stage(_In_ byte *dest) final;

	private:
		Vector4 transform;
		Vector4 factors;

		mutable bool dirty;
		mutable Matrix mdl;
		const Descriptor *height, *mask, *textures;
	};
}