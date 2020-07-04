#pragma once
#include <mutex>
#include "System.h"
#include "Physics/Objects/BVH.h"
#include "Physics/Objects/PhysicalObject.h"
#include "Physics/Properties/PhysicalProperties.h"

namespace Pu
{
	class MaterialDatabase;
	class MovementSystem;
	class ConstraintSystem;
	class SolverSystem;

	/* Defines the main entry point for all physics related code. */
	class PhysicalWorld2 final
		: public System
	{
	public:
		/* Initializes a new instance of a physical world system. */
		PhysicalWorld2(void);
		PhysicalWorld2(_In_ const PhysicalWorld2&) = delete;
		/* Move constructor. */
		PhysicalWorld2(_In_ PhysicalWorld2 &&value);
		/* Releases the resources allocated by the physical world. */
		~PhysicalWorld2(void)
		{
			Destroy();
		}

		_Check_return_ PhysicalWorld2& operator =(_In_ const PhysicalWorld2&) = delete;
		/* Move assignment. */
		_Check_return_ PhysicalWorld2& operator =(_In_ PhysicalWorld2 &&other);

		/* Adds a new static object to this world, with the specified parameters. */
		_Check_return_ PhysicsHandle AddStatic(_In_ const PhysicalObject &obj);
		/* Adds a new kinematic object to this world, with the specified parameters. */
		_Check_return_ PhysicsHandle AddKinematic(_In_ const PhysicalObject &obj);
		/* Adds the specified material to this world. */
		_Check_return_ PhysicsHandle AddMaterial(_In_ const PhysicalProperties &prop);
		/* Removes the specified object or material from this world. */
		void Destroy(_In_ PhysicsHandle handle);
		/* Gets the transform of the specified object. */
		_Check_return_ Matrix GetTransform(_In_ PhysicsHandle handle) const;
		/* Allows the user to visualize the physical world. */
		void Visualize(_In_ DebugRenderer &dbgRenderer, _In_ Vector3 camPos) const;

	protected:
		/* Updates the physical world. */
		void Update(_In_ float dt) final;

	private:
		friend class SolverSystem;
		friend class ConstraintSystem;

		MaterialDatabase *db;
		MovementSystem *sysMove;
		ConstraintSystem *sysCnst;
		SolverSystem *sysSolv;
		BVH searchTree;

		mutable std::mutex lock;
		vector<PhysicsHandle> handleLut;

		static void ThrowCorruptHandle(bool condition, const char *func);
		static void ValidatePhysicalObject(const PhysicalObject &obj);

		PhysicsHandle QueryPublicHandle(PhysicsHandle handle) const;
		uint16 QueryInternalIndex(PhysicsHandle handle) const;
		void ValidateHandle(PhysicsHandle handle) const;
		PhysicsHandle AddInternal(const PhysicalObject &obj, PhysicsType type);
		PhysicsHandle AllocPublicHandle(PhysicsType type, size_t idx);
		void DestroyInternal(PhysicsHandle hpublic, PhysicsHandle hinternal);
		void Destroy(void);
	};
}