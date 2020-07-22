#include "Physics/Objects/BVH.h"
#include "Physics/Systems/Raycasts.h"
#include "Physics/Systems/ShapeTests.h"
#include "Graphics/Diagnostics/DebugRenderer.h"
#include "Core/Collections/cstack.h"

#ifdef _DEBUG
#include <imgui/include/imgui.h>
#endif

#define BVH_HNULL				0xC000FFFF
#define BVH_INULL				0xFFFF
#define BVH_STACK_CAPACITY		0x40

#define is_leaf					pHandle) != BVH_HNULL
#define is_branch				pHandle) == BVH_HNULL
#define is_freed				Handle & PhysicsHandleBVHAllocBit
#define is_used					Handle ^ PhysicsHandleBVHAllocBit
#define get_depth				Handle >> 0x10 & 0xFF
#define pHandle					Handle & BVH_HNULL

static inline void set_depth(Pu::PhysicsHandle &handle, Pu::uint32 depth)
{
	handle &= 0xFF00FFFF;
	handle |= (depth & 0xFF) << 0x10;
}

/*
Node structure:
	currently 36 bytes
	Box:	Tight bounding box for static objects, slightly enlarged bounding box for kinematic or dynamic objects.
	Handle: The handle to the physics object associated with the leaf node, also contains flags.
	Parent: The index of the parent branch node; BVH_NULL if it is the root node.
	Child1: The index of the first child of this node; BVH_NULL if it's not set.
	Child2: The index of the second child of this node; BVH_NULL if it's not set.

Current allocation strategy:
	Free list, realloc if needed, free spaces are indicated with the PhysicsHandleBVHAllocBit flag.

SAH algorithm:
	Branch and Bound
*/

Pu::BVH::BVH(void)
	: root(BVH_INULL), count(0),
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
	const uint16 leafIdx = AllocLeaf(handle, box);
	if (count == 1)
	{
		nodes[leafIdx].Parent = BVH_INULL;
		root = leafIdx;
		return;
	}

	/* Find the best sibling for the new leaf. */
	const uint16 best = BestSibling(leafIdx);

	/* Create a new parent branch. */
	const uint16 oldParent = nodes[best].Parent;
	const uint16 newParent = AllocBranch();
	nodes[newParent].Parent = oldParent;
	nodes[newParent].Box = union_(box, nodes[best].Box);
	set_depth(nodes[newParent].Handle, (nodes[best].get_depth) + 1);

	/* We need to set the new parent as the root if the sibling was the old root. */
	if (oldParent != BVH_INULL)
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
	for (uint16 i = 0; i < capacity; i++)
	{
		const Node &node = nodes[i];

		if (node.is_used && (node.pHandle) == handle)
		{
			const uint16 oldParentIdx = nodes[i].Parent;

			/* Delete the leaf node. */
			FreeNode(i);

			if (oldParentIdx != BVH_INULL)
			{
				Node &oldParent = nodes[oldParentIdx];
				const uint16 sibling = oldParent.Child1 == i ? oldParent.Child2 : oldParent.Child1;

				/* Destroy the parent and connect the sibling to the grandparent. */
				const uint16 grandParentIdx = nodes[oldParentIdx].Parent;
				if (grandParentIdx != BVH_INULL)
				{
					if (nodes[grandParentIdx].Child1 == oldParentIdx) nodes[grandParentIdx].Child1 = sibling;
					else nodes[grandParentIdx].Child2 = sibling;

					nodes[sibling].Parent = grandParentIdx;
					FreeNode(oldParentIdx);

					/* Walk back up the treem refitting the bounding boxes. */
					Refit(grandParentIdx);
				}
				else
				{
					/* The sibling becomes the root node if no grandparent was available. */
					root = sibling;
					nodes[sibling].Parent = BVH_INULL;
					FreeNode(oldParentIdx);
				}
			}

			return;
		}
	}

	Log::Error("Unable to remove leaf node from BVH (handle wasn't found)!");
}

Pu::PhysicsHandle Pu::BVH::Raycast(Vector3 p, Vector3 d) const
{
	if (!count) return PhysicsNullHandle;
	const Vector3 rd = recip(d);

	/* Start at the root node. */
	cstack<uint16> stack{ BVH_STACK_CAPACITY };
	stack.push(root);

	/* Loop until we traversed the tree. */
	do
	{
		const uint16 i = stack.pop();

		/* Check if the branch (or leaf) overlaps. */
		if (raycast(p, rd, nodes[i].Box) >= 0.0f)
		{
			if ((nodes[i].is_leaf) return nodes[i].pHandle;
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
	if (!count) return;

	/* Start at the root node. */
	cstack<uint16> stack{ BVH_STACK_CAPACITY };
	stack.push(root);

	/* Loop until we traversed the tree. */
	do
	{
		const uint16 i = stack.pop();

		/* Check if the branch (or leaf) overlaps. */
		if (intersects(box, nodes[i].Box))
		{
			if ((nodes[i].is_leaf) result.emplace_back(nodes[i].pHandle);
			else
			{
				stack.push(nodes[i].Child1);
				stack.push(nodes[i].Child2);
			}
		}
	} while (stack.size());
}

void Pu::BVH::Frustumcast(const Frustum & frustum, vector<PhysicsHandle>& result) const
{
	if (!count) return;

	/* Start at the root node. */
	cstack<uint16> stack{ BVH_STACK_CAPACITY };
	stack.push(root);

	/* Loop until we traversed the tree. */
	do
	{
		const uint16 i = stack.pop();

		/* Check if the branch (or leaf) overlaps. */
		if (intersects_avx(frustum, nodes[i].Box))
		{
			if ((nodes[i].is_leaf) result.emplace_back(nodes[i].pHandle);
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

	for (uint16 i = 0; i < capacity; i++)
	{
		/* Skip any deallocated node. */
		if (nodes[i].is_freed) continue;

		/* The cost of the tree itself is the cost of all the internal nodes. */
		if ((nodes[i].is_branch) result += area(nodes[i].Box);
	}

	return result;
}

float Pu::BVH::GetEfficiency(void) const
{
	if (count < 1) return 0.0f;

	float sa = 0.0f;
	for (uint16 i = 0; i < capacity; i++)
	{
		/* Skip any deallocated nodes and sum up the area of the leaf nodes. */
		if (nodes[i].is_used) sa += area(nodes[i].Box);
	}

	return sa / area(nodes[root].Box);
}

#ifdef _DEBUG
void Pu::BVH::Visualize(DebugRenderer & renderer) const
{
	/* Display the stats in a seperate window. */
	if constexpr (ImGuiAvailable)
	{
		if (ImGui::Begin("BVH Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			const int32 rootDepth = count > 0 ? nodes[root].get_depth : 0;

			ImGui::Text("Leaf nodes:    %u", GetLeafCount());
			ImGui::Text("Total nodes:   %u", count);
			ImGui::Text("Depth:         %d", rootDepth);
			ImGui::Text("Memory:        %dKB", b2kb(sizeof(Node) * capacity));
			ImGui::Text("Cost:          %.f", GetTreeCost());
			ImGui::SliderInt("Display", reinterpret_cast<int*>(&displayDepth), 0, rootDepth);

			ImGui::End();
		}

		for (uint32 i = 0; i < capacity; i++)
		{
			const Node &node = nodes[i];
			if (node.is_used && (node.get_depth) == displayDepth) renderer.AddBox(nodes[i].Box, Color::Blue());
		}
	}
}
#endif

void Pu::BVH::Refit(uint16 start)
{
	for (uint16 i = start; i != BVH_INULL; i = nodes[i].Parent)
	{
		i = Balance(i);

		const Node &c1 = nodes[nodes[i].Child1];
		const Node &c2 = nodes[nodes[i].Child2];
		nodes[i].Box = union_(c1.Box, c2.Box);

		set_depth(nodes[i].Handle, 1 + max(c1.get_depth, c2.get_depth));
	}
}

Pu::uint16 Pu::BVH::Balance(uint16 idx)
{
	Node &a = nodes[idx];
	if ((a.is_leaf || (a.get_depth) < 2) return idx;

		uint16 iB = a.Child1;
		uint16 iC = a.Child2;

		Node &b = nodes[iB];
		Node &c = nodes[iC];

		int16 balance = (c.get_depth) - (b.get_depth);

		// Rotate C up
		if (balance > 1)
		{
			uint16 iF = c.Child1;
			uint16 iG = c.Child2;
			Node &f = nodes[iF];
			Node &g = nodes[iG];

			// Swap A and C
			c.Child1 = idx;
			c.Parent = a.Parent;
			a.Parent = iC;

			// A's old parent should point to C
			if (c.Parent != BVH_INULL)
			{
				if (nodes[c.Parent].Child1 == idx)
				{
					nodes[c.Parent].Child1 = iC;
				}
				else
				{
					nodes[c.Parent].Child2 = iC;
				}
			}
			else
			{
				root = iC;
			}

			// Rotate
			if ((f.get_depth) > (g.get_depth))
			{
				c.Child2 = iF;
				a.Child2 = iG;
				g.Parent = idx;
				a.Box = union_(b.Box, g.Box);
				c.Box = union_(a.Box, f.Box);

				set_depth(a.Handle, 1 + max(b.get_depth, g.get_depth));
				set_depth(c.Handle, 1 + max(a.get_depth, f.get_depth));
			}
			else
			{
				c.Child2 = iG;
				a.Child2 = iF;
				f.Parent = idx;
				a.Box = union_(b.Box, f.Box);
				c.Box = union_(a.Box, g.Box);

				set_depth(a.Handle, 1 + max(b.get_depth, f.get_depth));
				set_depth(c.Handle, 1 + max(a.get_depth, g.get_depth));
			}

			return iC;
		}

	// Rotate B up
	if (balance < -1)
	{
		uint16 iD = b.Child1;
		uint16 iE = b.Child2;
		Node &d = nodes[iD];
		Node &e = nodes[iE];

		// Swap A and B
		b.Child1 = idx;
		b.Parent = a.Parent;
		a.Parent = iB;

		// A's old parent should point to B
		if (b.Parent != BVH_INULL)
		{
			if (nodes[b.Parent].Child1 == idx)
			{
				nodes[b.Parent].Child1 = iB;
			}
			else
			{
				nodes[b.Parent].Child2 = iB;
			}
		}
		else
		{
			root = iB;
		}

		// Rotate
		if ((d.get_depth) > (e.get_depth))
		{
			b.Child2 = iD;
			a.Child1 = iE;
			e.Parent = idx;
			a.Box = union_(c.Box, e.Box);
			b.Box = union_(a.Box, d.Box);

			set_depth(a.Handle, 1 + max(c.get_depth, e.get_depth));
			set_depth(b.Handle, 1 + max(a.get_depth, d.get_depth));
		}
		else
		{
			b.Child2 = iE;
			a.Child1 = iD;
			d.Parent = idx;
			a.Box = union_(c.Box, d.Box);
			b.Box = union_(a.Box, e.Box);

			set_depth(a.Handle, 1 + max(c.get_depth, d.get_depth));
			set_depth(b.Handle, 1 + max(a.get_depth, e.get_depth));
		}

		return iB;
	}

	return idx;
}

Pu::uint16 Pu::BVH::BestSibling(uint16 node) const
{
	/* Start at the root node and descend down. */
	const AABB box = nodes[node].Box;
	uint16 i = root;

	do
	{
		const uint16 c1 = nodes[i].Child1;
		const uint16 c2 = nodes[i].Child2;
		const float a = area(nodes[i].Box);

		/* Calculate the direct cost, new cost and inherited cost. */
		const float dc = area(union_(nodes[i].Box, box));
		const float c = 2.0f * dc;
		const float ic = 2.0f * (dc - a);

		/* Calculate the cost of descending into the first child. */
		float cost1;
		if (c1 == BVH_INULL) cost1 = maxv<float>();
		else if ((nodes[c1].is_leaf) cost1 = area(union_(nodes[c1].Box, box));
		else
		{
			const float oldA = area(nodes[c1].Box);
			const float newA = area(union_(nodes[c1].Box, box));
			cost1 = (newA - oldA) + ic;
		}

		/* Calculate the cost of descending into the second child. */
		float cost2;
			if (c2 == BVH_INULL) cost2 = maxv<float>();
			else if ((nodes[c2].is_leaf) cost2 = area(union_(nodes[c2].Box, box));
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
	} while ((nodes[i].is_branch);

	return i;
}

Pu::uint16 Pu::BVH::AllocBranch(void)
{
	/* Check if there is an unused node that we can use. */
	if (count < capacity)
	{
		++count;

		for (uint16 i = 0; i < capacity; i++)
		{
			if (nodes[i].is_freed)
			{
				nodes[i].Handle = BVH_HNULL;
				return i;
			}
		}

		/* Should never occur. */
		Log::Fatal("BVH count corruption detected!");
	}
	else if (capacity >= BVH_INULL) Log::Fatal("Unable to allocate data for BVH Node!");

	/* Allocate more space for the node. */
	const size_t byteSize = ++capacity * sizeof(Node);
	nodes = reinterpret_cast<Node*>(realloc(nodes, byteSize));

	/* Clear the handle and return the index. */
	nodes[count].Handle = BVH_HNULL;
	return count++;
}

Pu::uint16 Pu::BVH::AllocLeaf(PhysicsHandle hobj, const AABB & box)
{
	const uint16 result = AllocBranch();
	nodes[result].Handle = hobj;
	nodes[result].Box = box;
	nodes[result].Child1 = BVH_INULL;
	nodes[result].Child2 = BVH_INULL;
	return result;
}

void Pu::BVH::FreeNode(uint16 idx)
{
	--count;
	if (idx == root) root = BVH_INULL;

	/*
	We just set a single bit, that indicates that this leaf node is no longer in use.
	Do a bit more on debug mode to easily indentify invalid operations.
	*/
#ifdef _DEBUG
	Node &node = nodes[idx];
	node.Handle = PhysicsHandleBVHAllocBit;
	node.Parent = BVH_INULL;
	node.Child1 = BVH_INULL;
	node.Child2 = BVH_INULL;
	node.Box = AABB();
#else
	nodes[idx].Handle = PhysicsHandleBVHAllocBit;
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