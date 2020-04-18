#pragma once
#include "Vector2.h"

namespace Pu
{
	/* Defines an object that contains a height field for a plane. */
	class HeightMap
	{
	public:
		/* Initializes a new instance of a heightmap with the width and height set to the specified value. */
		HeightMap(_In_ size_t dimensions, _In_ float scale);
		/* Initializes a new instance of a heightmap. */
		HeightMap(_In_ size_t width, _In_ size_t height, _In_ float patchSize);
		/* Copy constructor. */
		HeightMap(_In_ const HeightMap &value);
		/* Move constructor. */
		HeightMap(_In_ HeightMap &&value);
		/* Releases the resources allocated by the heightmap. */
		~HeightMap(void)
		{
			Free();
		}

		/* Copy assignment. */
		_Check_return_ HeightMap& operator =(_In_ const HeightMap &other);
		/* Move assignment. */
		_Check_return_ HeightMap& operator =(_In_ HeightMap &&other);

		/* Sets the height at a specific location in the heightmap. */
		void SetHeight(_In_ size_t x, _In_ size_t y, _In_ float value);
		/* Sets the height at a specific index in the heightmap. */
		void SetHeight(_In_ size_t i, _In_ float value);
		/* Gets whether the specified location is on the heightmap (scale not taken into acount). */
		_Check_return_ bool Contains(_In_ size_t x, _In_ size_t y) const;
		/* Gets whether the specified interpolated location is on the heightmap (scale is taken into account). */
		_Check_return_ bool Contains(_In_ Vector2 pos) const;
		/* Gets the height from a specific key point in the map (scale not taken into account). */
		_Check_return_ float GetHeight(_In_ size_t x, _In_ size_t y) const;
		/* Gets the height from an interpolated point in the map (scale is taken into account). */
		_Check_return_ float GetHeight(_In_ Vector2 pos) const;
		/* Attempts to get the height from a specific key point in the map (scale not taken into account). */
		_Check_return_ bool TryGetHeight(_In_ size_t x, _In_ size_t y, _Out_ float &height) const;
		/* Attempts to get the height from an interpolated point on the map (scale is taken into account). */
		_Check_return_ bool TryGetHeight(_In_ Vector2 pos, _Out_ float &height) const;

	private:
		float *data;
		size_t width, height;
		size_t boundX, boundY;
		float scale, iscale;

		void Alloc(void);
		void Copy(const float *other);
		void Free(void);
	};
}