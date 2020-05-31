#pragma once
#include "Vector3.h"

namespace Pu
{
	/* 
	Defines an object that contains a height field for a plane.
	The scale supplied as the contructor input refers to the scale between the heightmap vs the object it will be applied on,
	for instance: a heightmap is made with dimensions [1024, 1024].
	If it will be applied to a mesh with [16, 16] patches then the scale would be 64.
	*/
	class HeightMap
	{
	public:
		/* Initializes a new instance of a heightmap with the width and height set to the specified value. */
		HeightMap(_In_ size_t dimensions, _In_ float scale, _In_ bool addNormals);
		/* Initializes a new instance of a heightmap. */
		HeightMap(_In_ size_t width, _In_ size_t height, _In_ float scale, _In_ bool addNormals);
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
		/* Sets the normal at a specific location in the heightmap. */
		void SetNormal(_In_ size_t x, _In_ size_t y, _In_ Vector3 normal);
		/* Sets the height and normal at a specific location in the heightmap. */
		void SetHeightAndNormal(_In_ size_t x, _In_ size_t y, _In_ float height, _In_ Vector3 normal);
		/* Generates normals for every point on the heightmap. */
		void CalculateNormals(_In_ float displacement);
		/* Gets whether the specified location is on the heightmap. */
		_Check_return_ bool Contains(_In_ size_t x, _In_ size_t y) const;
		/* Gets whether the specified interpolated location is on the heightmap. */
		_Check_return_ bool Contains(_In_ Vector2 pos) const;
		/* Gets the height from a specific key point in the map. */
		_Check_return_ float GetHeight(_In_ size_t x, _In_ size_t y) const;
		/* Gets the height from an interpolated point in the map. */
		_Check_return_ float GetHeight(_In_ Vector2 pos) const;
		/* Gets the normal from a specific key point in the map. */
		_Check_return_ Vector3 GetNormal(_In_ size_t x, _In_ size_t y) const;
		/* Gets the normal from an interpolated point in the map. */
		_Check_return_ Vector3 GetNormal(_In_ Vector2 pos) const;
		/* Gets the height and normal from an interpolated point in the map. */
		void GetHeightAndNormal(_In_ Vector2 pos, _Out_ float &height, _Out_ Vector3 &normal) const;
		/* Attempts to get the height from a specific key point in the map. */
		_Check_return_ bool TryGetHeight(_In_ size_t x, _In_ size_t y, _Out_ float &height) const;
		/* Attempts to get the height from an interpolated point on the map. */
		_Check_return_ bool TryGetHeight(_In_ Vector2 pos, _Out_ float &height) const;
		/* Attempts to get the normal from an interpolated point on the map. */
		_Check_return_ bool TryGetNormal(_In_ Vector2 pos, _Out_ Vector3 &normal) const;
		/* Attempts to get the height and normal from an interpolated point on the map. */
		_Check_return_ bool TryGetHeightAndNormal(_In_ Vector2 pos, _Out_ float &height, _Out_ Vector3 &normal) const;

	private:
		float *data;
		Vector3 *normals;
		size_t width, height;
		size_t boundX, boundY;
		Vector2 patchSize, iPatchSize;

		void TransformPosition(Vector2 input, size_t &px, size_t &py, float &x, float &y) const;
		float QueryHeight(size_t apx, size_t apy, size_t bpx, size_t bpy, size_t cpx, size_t cpy, float t, float s) const;
		Vector3 QueryNormal(size_t apx, size_t apy, size_t bpx, size_t bpy, size_t cpx, size_t cpy, float t, float s) const;
		void Alloc(bool allocNormals);
		void Copy(const HeightMap &other);
		void Free(void);
	};
}