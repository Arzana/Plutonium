#pragma once
#include "PhysicsHandle.h"
#include "Core/Math/Shapes/AABB.h"
#include "Core/Math/Shapes/Frustum.h"
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
		/* Gets the objects that intersect with the specified frustum. */
		void Frustumcast(_In_ const Frustum &frustum, _Inout_ vector<PhysicsHandle> &result) const;

		/* Gets the cost of the internal branches of the BVH. */
		_Check_return_ float GetTreeCost(void) const;
		/* Gets the relative efficiency of the tree. */
		_Check_return_ float GetEfficiency(void) const;

		/* 
		Visualizes specific nodes in the BVH.
		The leaf nodes are the nodes that directly contain physical objects.
		The mid level is one level above the leaf nodes, so this first grouping level.
		The top level are all of the other branch nodes and the root node.
		*/
		void Visualize(_In_ DebugRenderer &renderer) const;

		/* Gets the amount of leaf nodes in this BVH. */
		_Check_return_ inline uint16 GetLeafCount(void) const
		{
			return (count + 1) / 2;
		}

	private:
		struct Node
		{
			AABB Box;
			PhysicsHandle Handle;
			uint16 Parent;
			uint16 Child1;
			uint16 Child2;
		};

		Node *nodes;
		uint16 count;
		uint16 capacity;
		uint16 root;

		mutable uint32 displayDepth;

		void Refit(uint16 start);
		uint16 Balance(uint16 idx);
		uint16 BestSibling(uint16 node) const;

		uint16 AllocBranch(void);
		uint16 AllocLeaf(PhysicsHandle hobj, const AABB &box);
		void FreeNode(uint16 idx);
		void CopyAlloc(const BVH &other);
		void Destroy(void);
	};
}