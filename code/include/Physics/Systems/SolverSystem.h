#pragma once
#include "MovementSystem.h"
#include "Physics/Objects/CollisionManifold.h"

namespace Pu
{
	/* Defines a system used to solve collision manifolds. */
	class SolverSystem
	{
	public:
		/* Initializes a new instance of a solver system that updates the specified movement system. */
		SolverSystem(_In_ MovementSystem &movement);
		SolverSystem(_In_ const SolverSystem &value) = delete;
		/* Move constructor. */
		SolverSystem(_In_ SolverSystem &&value);
		/* Releases the resources allocated by the solver system. */
		~SolverSystem(void)
		{
			Destroy();
		}

		_Check_return_ SolverSystem& operator =(_In_ const SolverSystem &other) = delete;
		/* Move assignment. */
		_Check_return_ SolverSystem& operator =(_In_ SolverSystem &&other);

		/* Adds a single item to the solver system, returns the index. */
		_Check_return_ size_t AddItem(_In_ const Matrix3 &MoI, _In_ float mass, _In_ float CoR, _In_ float CoF);
		/* Removes the item at the specified index. */
		void RemoveItem(_In_ size_t idx);
		/* Registers a collision event between two objects. */
		void RegisterCollision(_In_ const CollisionManifold &manifold);
		/* Solves all the collision events currently stored in the system and adds the impulses to the movement system. */
		void SolveConstriant(void);

	private:
		MovementSystem *movement;

		vector<Matrix3> moi;
		vector<float> imass;
		vector<Vector2> coefficients;
		vector<CollisionManifold> manifolds;

		ofloat *nx;
		ofloat *ny;
		ofloat *nz;
		int256 *hfirst;
		int256 *hsecond;
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

		size_t sharedCapacity;
		size_t kinematicCapacity;
		size_t resultCapacity;

		void EnsureBufferSize(size_t &staticCount, size_t &kinematicCount);
		void FillStatic(size_t staticCount);
		void FillKinematic(size_t &kinematicCount);
		void SolveStatic(size_t count);
		void SolveKinematic(size_t count);
		void Destroy(void);
	};
}