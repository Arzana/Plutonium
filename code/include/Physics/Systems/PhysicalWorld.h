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
	class ContactSystem;
	class ContactSolverSystem;

	/* Defines the main entry point for all physics related code. */
	class PhysicalWorld final
		: public System
	{
	public:
		/* Defines the amount of update sub-steps. */
		uint32 Substeps;

		/* Initializes a new instance of a physical world system. */
		PhysicalWorld(void);
		PhysicalWorld(_In_ const PhysicalWorld&) = delete;
		/* Move constructor. */
		PhysicalWorld(_In_ PhysicalWorld &&value);
		/* Releases the resources allocated by the physical world. */
		~PhysicalWorld(void)
		{
			Destroy();
		}

		_Check_return_ PhysicalWorld& operator =(_In_ const PhysicalWorld&) = delete;
		/* Move assignment. */
		_Check_return_ PhysicalWorld& operator =(_In_ PhysicalWorld &&other);

		/* Adds a new static object to this world, with the specified parameters. */
		_Check_return_ PhysicsHandle AddStatic(_In_ const PhysicalObject &obj);
		/* Adds a new kinematic object to this world, with the specified parameters. */
		_Check_return_ PhysicsHandle AddKinematic(_In_ const PhysicalObject &obj);
		/* Adds the specified material to this world. */
		_Check_return_ PhysicsHandle AddMaterial(_In_ const PhysicalProperties &prop);
		/* Sets the gravitational constant. */
		void SetGravity(_In_ Vector3 g);
		/* Removes the specified object or material from this world. */
		void Destroy(_In_ PhysicsHandle handle);
		/* Gets the transform of the specified object. */
		_Check_return_ Matrix GetTransform(_In_ PhysicsHandle handle) const;
		/* Allows the user to visualize the physical world. */
		void Visualize(_In_ DebugRenderer &dbgRenderer, _In_ Vector3 camPos, _In_ float dt) const;

	protected:
		/* Updates the physical world. */
		void Update(_In_ float dt) final;

	private:
		friend class ContactSolverSystem;
		friend class ContactSystem;

		MaterialDatabase *db;
		MovementSystem *sysMove;
		ContactSystem *sysCnst;
		ContactSolverSystem *sysSolv;
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