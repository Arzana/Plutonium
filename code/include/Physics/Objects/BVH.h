#pragma once
#include "PhysicsHandle.h"
#include "Core/Math/Shapes/AABB.h"
#include "Core/Collections/vector.h"

namespace Pu
{
	class DebugRenderer;

	/* Defines a dynamic BVH used for physics. */
	class BVH
	{
	public:
		/* Initializes an empty instance of a BVH. */
		BVH(void);
		/* Copy constructor. */
		BVH(_In_ const BVH &value);
		/* Move constructor. */
		BVH(_In_ BVH &&value);
		/* Releases the resources allocated by the BVH. */
		~BVH(void)
		{
			Destroy();
		}

		/* Copy assignment. */
		_Check_return_ BVH& operator =(_In_ const BVH &other);
		/* Move assignment. */
		_Check_return_ BVH& operator =(_In_ BVH &&other);

		/* Inserts a new object into this BVH. */
		void Insert(_In_ PhysicsHandle handle, _In_ const AABB &box);
		/* Removes the specified object from this BVH. */
		void Remove(_In_ PhysicsHandle handle);

		/* Performs a basic raycast against the BVH, returns the object hit. */
		_Check_return_ PhysicsHandle Raycast(_In_ Vector3 p, _In_ Vector3 d) const;
		/* Gets the objects that overlap with the specified bounding box. */
		void Boxcast(_In_ const AABB &box, _Inout_ vector<PhysicsHandle> &result) const;
		/* Gets the cost of the internal branches of the BVH. */
		_Check_return_ float GetTreeCost(void) const;
		/* Visualizes the BVH. */
		void Visualize(_In_ DebugRenderer &renderer) const;

	private:
		struct Node
		{
			AABB Box;
			PhysicsHandle Handle;
			uint32 Parent;
			uint32 Child1;
			uint32 Child2;
		};

		Node *nodes;
		uint32 count;
		uint32 capacity;
		uint32 root;

		void Refit(uint32 start);
		uint32 BestSibling(uint32 node) const;

		uint32 AllocBranch(void);
		uint32 AllocLeaf(PhysicsHandle hobj, const AABB &box);
		void FreeNode(uint32 idx);
		void CopyAlloc(const BVH &other);
		void Destroy(void);
	};
}