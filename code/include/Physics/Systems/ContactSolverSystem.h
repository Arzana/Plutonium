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
		ContactSolverSystem(_In_ ContactSolverSystem &&value) = delete;
		/* Releases the resources allocated by the solver system. */
		~ContactSolverSystem(void)
		{
			Destroy();
		}

		_Check_return_ ContactSolverSystem& operator =(_In_ const ContactSolverSystem &other) = delete;
		_Check_return_ ContactSolverSystem& operator =(_In_ ContactSolverSystem &&other) = delete;

		/* Adds a single item to the solver system, returns the index. */
		void AddItem(_In_ PhysicsHandle handle, _In_ const Matrix3 &iMoI, _In_ float imass, _In_ float CoR, _In_ float CoF);
		/* Removes the item at the specified index. */
		void RemoveItem(_In_ PhysicsHandle handle);
		/* Solves all the collision events currently stored in the system and adds the impulses to the movement system. */
		void SolveConstriants(void);

	private:
		PhysicalWorld *world;

		std::map<PhysicsHandle, Matrix3> imoi;
		std::map<PhysicsHandle, float> imass;
		std::map<PhysicsHandle, Vector2> coefficients;

		ofloat *cor1;
		ofloat *cor2;
		ofloat *cof1;
		ofloat *cof2;
		ofloat *px1;
		ofloat *py1;
		ofloat *pz1;
		ofloat *px2;
		ofloat *py2;
		ofloat *pz2;
		ofloat *vx1;
		ofloat *vy1;
		ofloat *vz1;
		ofloat *vx2;
		ofloat *vy2;
		ofloat *vz2;
		ofloat *wp1;
		ofloat *wy1;
		ofloat *wr1;
		ofloat *wp2;
		ofloat *wy2;
		ofloat *wr2;
		ofloat *imass1;
		ofloat *imass2;
		ofloat *m001;
		ofloat *m011;
		ofloat *m021;
		ofloat *m101;
		ofloat *m111;
		ofloat *m121;
		ofloat *m201;
		ofloat *m211;
		ofloat *m221;
		ofloat *m002;
		ofloat *m012;
		ofloat *m022;
		ofloat *m102;
		ofloat *m112;
		ofloat *m122;
		ofloat *m202;
		ofloat *m212;
		ofloat *m222;

		ofloat *jx;
		ofloat *jy;
		ofloat *jz;
		ofloat *jpitch;
		ofloat *jyaw;
		ofloat *jroll;

		size_t capacity;

		void EnsureBufferSize(void);
		void FillBuffers(void);
		void VectorSolve(void);
		void ApplyImpulses(void);
		void Destroy(void);
	};
}