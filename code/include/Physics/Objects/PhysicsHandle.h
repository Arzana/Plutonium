#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/*
	Defines the handle to a physical object.

	Physical handles store various pieces of fast information.
	[TTUUUUUU UUUUUUUU IIIIIIII IIIIIIII]
	T (2-bits):		The type of the object, also the PhysicalWorld vector in which it is stored.
	U (14-bits):	Implementation bits, these bits can be used for internal storage, note that these need to be cleared when passing to other systems.
	I (16-bits):	The index in the lookup vector, used to determine the actual index.

	--------------------BVH Implementation--------------------
	[TTA00000 DDDDDDDD IIIIIIII IIIIIIII]
	A (1-bit):		Allocation flag, denotes recycled nodes.
	D (8-bits):		BVH node depth.
	*/
	using PhysicsHandle = uint32;
	/* Defines the handle used to denote a null handle. */
	constexpr PhysicsHandle PhysicsNullHandle = ~0u;
	/* Defines a bit-mask for accessing the implementation bits. */
	constexpr uint32 PhysicsHandleImplBits = 0x1FFF0000;
	/* Defines a bit-mask for the BVH allocation flag. */
	constexpr uint32 PhysicsHandleBVHAllocBit = 0x20000000;

	/* Defines the types of physics objects. */
	enum class PhysicsType : uint8
	{
		/* Inifite plane (depricated). */
		Plane,
		/* Physical material. */
		Material,
		/* Unmovable object. */
		Static,
		/* Full physics object. */
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