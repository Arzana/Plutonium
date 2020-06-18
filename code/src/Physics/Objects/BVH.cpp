#include "Physics/Objects/BVH.h"
#include "Physics/Systems/Raycasts.h"
#include "Physics/Systems/ShapeTests.h"
#include "Core/Diagnostics/Profiler.h"
#include "Core/Diagnostics/Logging.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include <imgui/include/imgui.h>
#include <stack>
#include <queue>
#include <set>

#define BVH_NULL		0xC0FFFFFF

#define is_leaf(node)	(node.Handle != BVH_NULL)
#define is_branch(node)	(node.Handle == BVH_NULL)
#define is_freed(node)	(node.Handle & PhysicsHandleBVHAllocBit)
#define is_used(node)	(node.Handle ^ PhysicsHandleBVHAllocBit)

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
		if (nodes[i].Handle == handle && is_used(nodes[i]))
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
			if (is_leaf(nodes[i])) return nodes[i].Handle;
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
			if (is_leaf(nodes[i])) result.emplace_back(nodes[i].Handle);
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
		if (is_freed(nodes[i])) continue;

		/* The cost of the tree itself is the cost of all the internal nodes. */
		if (is_branch(nodes[i])) result += area(nodes[i].Box);
	}

	return result;
}

float Pu::BVH::GetEfficiency(void) const
{
	if (count < 1) return 0.0f;

	float sa = 0.0f;
	for (uint32 i = 0; i < capacity; i++)
	{
		/* Skip any deallocated nodes and sum up the area of the leaf nodes. */
		if (is_used(nodes[i])) sa += area(nodes[i].Box);
	}

	return sa / area(nodes[root].Box);
}

void Pu::BVH::Visualize(DebugRenderer & renderer, bool leafs, bool midLevel, bool top) const
{
	Profiler::BeginDebug();

	std::set<uint32> mid;
	vector<uint32> midOrTop;

	for (uint32 i = 0; i < capacity; i++)
	{
		/* Skip any deallocated nodes. */
		const Node &node = nodes[i];
		if (is_freed(node)) continue;

		if (is_leaf(node))
		{
			/* Handle leaf nodes. */
			if (leafs) renderer.AddBox(node.Box, Color::Red());
			if (node.Parent != BVH_NULL && midLevel) mid.emplace(node.Parent);
		}
		else if (!midLevel)
		{
			/* Render the top level directly if the mid level isn't being rendered. */
			if (top) renderer.AddBox(node.Box, Color::Green());
		}
		else midOrTop.emplace_back(i);
	}

	/* Remove all of the mid level nodes from the mid/top vector. */
	for (uint32 i = 0; i < midOrTop.size(); i++)
	{
		if (mid.find(midOrTop[i]) != mid.end()) midOrTop.removeAt(i);
	}

	/* Render all the mid level nodes. */
	for (uint32 i : mid) renderer.AddBox(nodes[i].Box, Color::Blue());

	/* Render the top level nodes if requested. */
	if (midLevel && top)
	{
		for (uint32 i : midOrTop) renderer.AddBox(nodes[i].Box, Color::Green());
	}

	/* Display the stats in a seperate window. */
	if constexpr (ImGuiAvailable)
	{
		if (ImGui::Begin("BVH Statistics"))
		{
			ImGui::Text("Leaf nodes:    %u", GetLeafCount());
			ImGui::Text("Total nodes:   %u", count);
			ImGui::Text("Memory:        %dKB", b2kb(sizeof(Node) * capacity));
			ImGui::Text("Cost:          %.f", GetTreeCost());
			ImGui::Text("Efficiency:    %.f%%", GetEfficiency());

			ImGui::End();
		}
	}

	Profiler::End();
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
	/* Start at the root node and descend down. */
	const AABB box = nodes[node].Box;
	uint32 i = root;

	do
	{
		const uint32 c1 = nodes[i].Child1;
		const uint32 c2 = nodes[i].Child2;
		const float a = area(nodes[i].Box);

		/* Calculate the direct cost, new cost and inherited cost. */
		const float dc = area(union_(nodes[i].Box, box));
		const float c = 2.0f * dc;
		const float ic = 2.0f * (dc - a);

		/* Calculate the cost of descending into the first child. */
		float cost1;
		if (c1 == BVH_NULL) cost1 = maxv<float>();
		else if (is_leaf(nodes[c1])) cost1 = area(union_(nodes[c1].Box, box));
		else
		{
			const float oldA = area(nodes[c1].Box);
			const float newA = area(union_(nodes[c1].Box, box));
			cost1 = (newA - oldA) + ic;
		}

		/* Calculate the cost of descending into the second child. */
		float cost2;
		if (c2 == BVH_NULL) cost2 = maxv<float>();
		else if (is_leaf(nodes[c2])) cost2 = area(union_(nodes[c2].Box, box));
		else
		{
			const float oldA = area(nodes[c2].Box);
			const float newA = area(union_(nodes[c2].Box, box));
			cost2 = (newA - oldA) + ic;
		}

		/* Stop descending if needed. */
		if (c < cost1 && c < cost2) break;

		/* Descend further down. */
		i = c1 < c2 ? c1 : c2;
	} while (is_branch(nodes[i]));

	return i;
}

Pu::uint32 Pu::BVH::AllocBranch(void)
{
	/* Check if there is an unused node that we can use. */
	if (count < capacity)
	{
		++count;

		for (uint32 i = 0; i < capacity; i++)
		{
			if (is_freed(nodes[i]))
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