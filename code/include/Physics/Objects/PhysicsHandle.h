#pragma once
#include <utility>
#include "Core/Math/Constants.h"

namespace Pu
{
	/*
	Defines the handle to a physical object.
	Any part of the handle can be ignored if not needed and the bits can be used for other purposes,
	but the public handle must adhere to the following format. Any bits marked as zero should be zero.

	Physical handles store various pieces of fast information.
	[TTTT00UU UUUUUUUU IIIIIIII IIIIIIII]
	T (4-bits):		The type of the object, also the PhysicalWorld vector in which it is stored.
	U (10-bits):	Implementation bits, these bits can be used for internal storage, note that these need to be cleared when passing to other systems.
	I (16-bits):	The index in the lookup vector, used to determine the actual index.

	--------------------BVH Implementation--------------------
	[TTTT000A DDDDDDDD IIIIIIII IIIIIIII]
	A (1-bit):		Allocation flag, denotes recycled nodes.
	D (8-bits):		BVH node depth.

	----------------Constraint Implementation-----------------
	[TTTT00SE 00000000 IIIIIIII IIIIIIII]
	S (1-bit):		Skip bit, if this bit is set, then the contraint system will ignore collisions with this object.
	E (1-bit):		Event bit, if anything collides with this object it will trigger an event instead of being solved.

	----------------Rendering Implementation------------------
	[TTTT0000 0000SSSS IIIIIIII IIIIIIII]
	S (4-bits):		Subpass index, defines in which subpass specific objects should be rendered.
	*/
	using PhysicsHandle = uint32;
	/* Defines a pair of physics handles. */
	using PhysicsHandlePair = std::pair<PhysicsHandle, PhysicsHandle>;

	/* Defines the handle used to denote a null handle. */
	constexpr PhysicsHandle PhysicsNullHandle = 0u;
	/* Defines a bit-mask for accessing the implementation bits. */
	constexpr uint32 PhysicsHandleImplBits = 0x7FF0000;
	/* Defines a bit-mask for accessing the event bit. */
	constexpr uint32 PhysicsHandleEventBit = 0x1000000;
	/* Defines a bit-mask for accessing the skip bit. */
	constexpr uint32 PhysicsHandleSkipBit = 0x2000000;
	/* Defines a bit-mask for the BVH allocation flag. */
	constexpr uint32 PhysicsHandleBVHAllocBit = 0x1000000;

	/* Defines the types of physics objects. */
	enum class PhysicsType : uint8
	{
		/* Physical material. */
		Material = 0,
		/* Unmovable object. */
		Static = 1,
		/* Full physics object. */
		Kinematic = 2,
		/* Partial physics object. */
		Dynamic = 3,
		/* Purely visual light source. */
		LightSource = 4
	};

	/* Gets the index of the object associated with the handle. */
	_Check_return_ static constexpr inline uint16 physics_get_lookup_id(_In_ PhysicsHandle handle)
	{
		return handle & 0xFFFF;
	}

	/* Gets the list (or type) of the object associated with the handle. */
	_Check_return_ static constexpr inline PhysicsType physics_get_type(_In_ PhysicsHandle handle)
	{
		return static_cast<PhysicsType>(handle >> 28);
	}

	/* Creates a new physics handle. */
	_Check_return_ static constexpr inline PhysicsHandle create_physics_handle(_In_ PhysicsType type, _In_ uint16 idx)
	{
		return static_cast<uint32>(type) << 28 | idx;
	}

	/* Creates a new physics handle (truncates). */
	_Check_return_ static constexpr inline PhysicsHandle create_physics_handle(_In_ PhysicsType type, _In_ size_t idx)
	{
		return static_cast<uint32>(type) << 28 | static_cast<uint16>(idx);
	}
}