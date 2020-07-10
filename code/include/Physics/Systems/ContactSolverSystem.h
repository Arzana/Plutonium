#pragma once
#include <map>
#include "Physics/Objects/PhysicsHandle.h"
#include "Core/Math/Matrix3.h"

namespace Pu
{
	class PhysicalWorld;

	/* Defines a system used to solve collision manifolds. */
	class ContactSolverSystem
	{
	public:
		/* Initializes a new instance of a solver system that updates the specified movement system. */
		ContactSolverSystem(_In_ PhysicalWorld &world);
		ContactSolverSystem(_In_ const ContactSolverSystem &value) = delete;
		/* Move constructor. */
		ContactSolverSystem(_In_ ContactSolverSystem &&value);
		/* Releases the resources allocated by the solver system. */
		~ContactSolverSystem(void)
		{
			Destroy();
		}

		_Check_return_ ContactSolverSystem& operator =(_In_ const ContactSolverSystem &other) = delete;
		/* Move assignment. */
		_Check_return_ ContactSolverSystem& operator =(_In_ ContactSolverSystem &&other);

		/* Adds a single item to the solver system, returns the index. */
		void AddItem(_In_ PhysicsHandle handle, _In_ const Matrix3 &MoI, _In_ float mass, _In_ float CoR, _In_ float CoF);
		/* Removes the item at the specified index. */
		void RemoveItem(_In_ PhysicsHandle handle);
		/* Solves all the collision events currently stored in the system and adds the impulses to the movement system. */
		void SolveConstriants(void);

	private:
		PhysicalWorld *world;

		std::map<PhysicsHandle, Matrix3> moi;
		std::map<PhysicsHandle, float> imass;
		std::map<PhysicsHandle, Vector2> coefficients;

		ofloat *fcor;
		ofloat *scor;
		ofloat *fcof;
		ofloat *scof;
		ofloat *vx;
		ofloat *vy;
		ofloat *vz;
		ofloat *fimass;
		ofloat *simass;

		ofloat *jx;
		ofloat *jy;
		ofloat *jz;

		size_t capacity;

		void EnsureBufferSize(void);
		void FillBuffers(void);
		void VectorSolve(void);
		void ApplyImpulses(void);
		void Destroy(void);
	};
}