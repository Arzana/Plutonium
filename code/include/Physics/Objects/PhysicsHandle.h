#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/*
	Defines the handle to a physical object.

	Physical handles store various pieces of fast information.
	[TTA00000 00000000 IIIIIIII IIIIIIII]

	T (2-bits):		The type of the object, also the PhysicalWorld vector in which it is stored.
	A (1-bit):		Allocation flag, used by the BVH to set whether the node is in use.
	I (16-bits):	The index in the lookup vector, used to determine the actual index.
	*/
	using PhysicsHandle = uint32;
	/* Defines the handle used to denote a null handle. */
	constexpr PhysicsHandle PhysicsNullHandle = ~0u;
	/* Defines a bit-mask for the BVH allocation flag. */
	constexpr uint32 PhysicsHandleBVHAllocBit = 0x20000000;

	/* Defines the types of physics objects. */
	enum class PhysicsType : uint8
	{
		Plane,
		Static,
		Kinematic
	};

	/* Gets the index of the object associated with the handle. */
	_Check_return_ static constexpr inline uint16 physics_get_lookup_id(_In_ PhysicsHandle handle)
	{
		return handle & 0xFFFF;
	}

	/* Gets the list (or type) of the object associated with the handle. */
	_Check_return_ static constexpr inline PhysicsType physics_get_type(_In_ PhysicsHandle handle)
	{
		return static_cast<PhysicsType>(handle >> 30);
	}

	/* Creates a new physics handle. */
	_Check_return_ static constexpr inline PhysicsHandle create_physics_handle(_In_ PhysicsType type, _In_ uint16 idx)
	{
		return static_cast<uint32>(type) << 30 | idx;
	}

	/* Creates a new physics handle (truncates). */
	_Check_return_ static constexpr inline PhysicsHandle create_physics_handle(_In_ PhysicsType type, _In_ size_t idx)
	{
		return static_cast<uint32>(type) << 30 | static_cast<uint16>(idx);
	}
}