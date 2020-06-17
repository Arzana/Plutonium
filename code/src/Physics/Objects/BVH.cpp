#include "Physics/Objects/BVH.h"
#include "Physics/Systems/Raycasts.h"
#include "Physics/Systems/ShapeTests.h"
#include "Core/Diagnostics/Logging.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include <stack>
#include <queue>

#define BVH_NULL		0xC0FFFFFF

/*
Node structure:
	currently 40 bytes
	Box:	Tight bounding box for static objects, slightly enlarged bounding box for kinematic or dynamic objects.
	Handle: The handle to the physics object associated with the leaf node, also contains flags.
	Parent: The index of the parent branch node; BVH_NULL if it is the root node.
	Child1: The index of the first child of this node; BVH_NULL if it's not set.
	Child2: The index of the second child of this node; BVH_NULL if it's not set.

Current allocation strategy:
	Free list, realloc if needed, free spaces are indicated with the BVH_ALLOC_BIT flag.

SAH algorithm:
	Branch and Bound
*/

Pu::BVH::BVH(void)
	: root(BVH_NULL), count(0),
	capacity(0), nodes(nullptr)
{}

Pu::BVH::BVH(const BVH & value)
	: root(value.root), count(value.count), capacity(value.capacity)
{
	CopyAlloc(value);
}

Pu::BVH::BVH(BVH && value)
	: root(value.root), nodes(value.nodes),
	count(value.count), capacity(value.capacity)
{
	value.nodes = nullptr;
}

Pu::BVH & Pu::BVH::operator=(const BVH & other)
{
	if (this != &other)
	{
		root = other.root;
		count = other.count;
		capacity = other.capacity;
		CopyAlloc(other);
	}

	return *this;
}

Pu::BVH & Pu::BVH::operator=(BVH && other)
{
	if (this != &other)
	{
		Destroy();

		root = other.root;
		count = other.count;
		capacity = other.capacity;
		nodes = other.nodes;

		other.nodes = nullptr;
	}

	return *this;
}

void Pu::BVH::Insert(PhysicsHandle handle, const AABB & box)
{
	/* Add the leaf to the buffer and check if it't the root. */
	const uint32 leafIdx = AllocLeaf(handle, box);
	if (count == 1)
	{
		nodes[leafIdx].Parent = BVH_NULL;
		root = leafIdx;
		return;
	}

	/* Find the best sibling for the new leaf. */
	const uint32 best = BestSibling(leafIdx);

	/* Create a new parent branch. */
	const uint32 oldParent = nodes[best].Parent;
	const uint32 newParent = AllocBranch();
	nodes[newParent].Parent = oldParent;
	nodes[newParent].Box = union_(box, nodes[best].Box);

	/* We need to set the new parent as the root if the sibling was the old root. */
	if (oldParent != BVH_NULL)
	{
		if (nodes[oldParent].Child1 == best) nodes[oldParent].Child1 = newParent;
		else nodes[oldParent].Child2 = newParent;
	}
	else root = newParent;

	nodes[newParent].Child1 = best;
	nodes[newParent].Child2 = leafIdx;
	nodes[best].Parent = newParent;
	nodes[leafIdx].Parent = newParent;

	/* Walk back up the tree, refitting the bounding boxes. */
	Refit(newParent);
}

void Pu::BVH::Remove(PhysicsHandle handle)
{
	/* Find the leaf node associated with this handle. */
	for (uint32 i = 0; i < capacity; i++)
	{
		if (nodes[i].Handle == handle && !(nodes[i].Handle & PhysicsHandleBVHAllocBit))
		{
			const uint32 oldParentIdx = nodes[i].Parent;

			/* Delete the leaf node. */
			FreeNode(i);

			if (oldParentIdx != BVH_NULL)
			{
				/* Move the sibling over to the parents location in the tree. */
				Node &oldParent = nodes[oldParentIdx];
				if (oldParent.Child1 == i) nodes[oldParent.Child2].Parent = oldParent.Parent;
				else nodes[oldParent.Child1].Parent = oldParent.Parent;

				/* Walk back up the treem refitting the bounding boxes. */
				Refit(oldParent.Parent);
			}

			return;
		}
	}
}

Pu::PhysicsHandle Pu::BVH::Raycast(Vector3 p, Vector3 d) const
{
	const Vector3 rd = recip(d);

	/* Start at the root node. */
	std::stack<uint32> stack;
	stack.push(root);

	/* Loop until we traversed the tree. */
	do
	{
		const uint32 i = stack.top();
		stack.pop();

		/* Check if the branch (or leaf) overlaps. */
		if (raycast(p, rd, nodes[i].Box) >= 0.0f)
		{
			if (nodes[i].Handle != BVH_NULL) return nodes[i].Handle;
			else
			{
				stack.push(nodes[i].Child1);
				stack.push(nodes[i].Child2);
			}
		}
	} while (stack.size());

	/* Nothing was lit by the ray. */
	return PhysicsNullHandle;
}

void Pu::BVH::Boxcast(const AABB & box, vector<PhysicsHandle>& result) const
{
	/* Start at the root node. */
	std::stack<uint32> stack;
	stack.push(root);

	/* Loop until we traversed the tree. */
	do
	{
		const uint32 i = stack.top();
		stack.pop();
		
		/* Check if the branch (or leaf) overlaps. */
		if (intersects(box, nodes[i].Box))
		{
			if (nodes[i].Handle != BVH_NULL) result.emplace_back(nodes[i].Handle);
			else
			{
				stack.push(nodes[i].Child1);
				stack.push(nodes[i].Child2);
			}
		}

	} while (stack.size());
}

float Pu::BVH::GetTreeCost(void) const
{
	float result = 0.0f;

	for (uint32 i = 0; i < capacity; i++)
	{
		/* Skip any deallocated node. */
		if (nodes[i].Handle & PhysicsHandleBVHAllocBit) continue;

		/* The cost of the tree itself is the cost of all the internal nodes. */
		if (nodes[i].Handle != BVH_NULL) result += nodes[i].Box.GetArea();
	}

	return result;
}

void Pu::BVH::Visualize(DebugRenderer & renderer) const
{
	for (uint32 i = 0; i < capacity; i++)
	{
		const Node &node = nodes[i];
		if (node.Handle & PhysicsHandleBVHAllocBit) continue;

		if (node.Handle == BVH_NULL) renderer.AddBox(node.Box, Color::Red());
		else renderer.AddBox(node.Box, Color::Cyan());
	}
}

void Pu::BVH::Refit(uint32 start)
{
	for (uint32 i = start; i != BVH_NULL; i = nodes[i].Parent)
	{
		const AABB &b1 = nodes[nodes[i].Child1].Box;
		const AABB &b2 = nodes[nodes[i].Child2].Box;
		nodes[i].Box = union_(b1, b2);
	}
}

Pu::uint32 Pu::BVH::BestSibling(uint32 node) const
{
	/* Precalculate some values. */
	const AABB box = nodes[node].Box;
	const float l = box.GetArea();

	/* Define the best sibling and the best cost so far. */
	uint32 bs = root;
	float bc = nodes[root].Box.GetArea();

	/* Initialize the priority queue. */
	std::queue<std::pair<uint32, float>> priority;
	priority.push(std::make_pair(bs, 0.0f));

	do
	{
		auto[i, ic] = priority.front();
		priority.pop();

		/* Calculate the direct cost and the cost of the entire node. */
		const float dc = union_(box, nodes[i].Box).GetArea();
		const float nc = dc + ic;
		
		/* Check if this node is better than or previous best. */
		if (nc < bc)
		{
			bc = nc;
			bs = i;
			
			if (nodes[i].Child1 != BVH_NULL)
			{
				/* Check if we should go into the subtree. */
				const float sa = nodes[i].Box.GetArea();
				const float lc = l + ic + (nc - bc);
				if (lc < bc)
				{
					ic += sa;
					priority.push(std::make_pair(nodes[i].Child1, ic));
					priority.push(std::make_pair(nodes[i].Child2, ic));
				}
			}
		}

	} while (priority.size());

	return bs;
}

Pu::uint32 Pu::BVH::AllocBranch(void)
{
	/* Check if there is an unused node that we can use. */
	if (count < capacity)
	{
		++count; 

		for (uint32 i = 0; i < capacity; i++) 
		{
			if (nodes[i].Handle & PhysicsHandleBVHAllocBit)
			{
				nodes[i].Handle = BVH_NULL;
				return i;
			}
		}

		/* Should never occur. */
		Log::Fatal("BVH count corruption detected!");
	}

	/* Allocate more space for the node. */
	const size_t byteSize = ++capacity * sizeof(Node);
	nodes = reinterpret_cast<Node*>(realloc(nodes, byteSize));

	/* Clear the handle and return the index. */
	nodes[count].Handle = BVH_NULL;
	return count++;
}

Pu::uint32 Pu::BVH::AllocLeaf(PhysicsHandle hobj, const AABB & box)
{
	const uint32 result = AllocBranch();
	nodes[result].Handle = hobj;
	nodes[result].Box = box;
	nodes[result].Child1 = BVH_NULL;
	nodes[result].Child2 = BVH_NULL;
	return result;
}

void Pu::BVH::FreeNode(uint32 idx)
{
	--count;

	/* 
	We just set a single bit, that indicates that this leaf node is no longer in use.
	Do a bit more on debug mode to easily indentify invalid operations.
	*/
#ifdef _DEBUG
	Node &node = nodes[idx];
	node.Handle = PhysicsHandleBVHAllocBit;
	node.Parent = BVH_NULL;
	node.Child1 = BVH_NULL;
	node.Child2 = BVH_NULL;
	node.Box = AABB();
#else
	nodes[idx].Handle = BVH_ALLOC_BIT;
#endif
}

void Pu::BVH::CopyAlloc(const BVH & other)
{
	const size_t byteSize = other.capacity * sizeof(Node);
	nodes = reinterpret_cast<Node*>(realloc(nodes, byteSize));
	memcpy(nodes, other.nodes, byteSize);
}

void Pu::BVH::Destroy(void)
{
	if (nodes) free(nodes);
}