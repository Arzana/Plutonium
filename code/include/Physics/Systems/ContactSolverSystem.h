#pragma once
#include <map>
#include "Physics/Objects/PhysicsHandle.h"
#include "Physics/Properties/MechanicalProperties.h"
#include "Core/Math/Matrix3.h"

#ifdef _DEBUG
#include "Core/Time.h"
#endif

namespace Pu
{
	class PhysicalWorld;
	class DebugRenderer;

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
		void AddItem(_In_ PhysicsHandle handle, _In_ const Matrix3 &iMoI, _In_ float imass, _In_ const MechanicalProperties &props);
		/* Removes the item at the specified index. */
		void RemoveItem(_In_ PhysicsHandle handle);
		/* Solves all the collision events currently stored in the system and adds the impulses to the movement system. */
		void SolveConstriants(_In_ ofloat dt);

#ifdef _DEBUG
		/* Visualizes the forces being applied to kinematic objects. */
		void Visualize(_In_ DebugRenderer &dbgRenderer) const;
#endif

	private:
		PhysicalWorld *world;
		size_t capacity;

		std::map<PhysicsHandle, Matrix3> imoi;
		std::map<PhysicsHandle, float> imass;
		std::map<PhysicsHandle, MechanicalProperties> coefficients;

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

#ifdef _DEBUG
		struct TimedForce
		{
			pu_clock::time_point Time;
			float Magnitude;
			Vector3 At;
			Vector3 Dir;
		};

		mutable vector<TimedForce> appliedForces;
#endif

		void EnsureBufferSize(void);
		void FillBuffers(void);
		void VectorSolve(ofloat dt);
		void ApplyImpulses(void);
		void Destroy(void);
	};
}