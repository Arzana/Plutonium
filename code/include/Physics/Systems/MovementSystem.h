#pragma once
#include "Physics/Objects/PhysicsHandle.h"
#include "Core/Collections/simd_vector.h"
#include "Core/Math/VectorSSE.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	class DebugRenderer;

	/* Defines a system that handles the integration of position. */
	class MovementSystem
	{
	public:
		/* Initializes a new instance of a movement system. */
		MovementSystem(void);
		MovementSystem(_In_ const MovementSystem &value) = delete;
		/* Move constructor. */
		MovementSystem(_In_ MovementSystem &&value) = default;

		_Check_return_ MovementSystem& operator =(_In_ const MovementSystem &other) = delete;
		/* Move assignment. */
		_Check_return_ MovementSystem& operator =(_In_ MovementSystem &&other) = default;

		/* Sets the gravitational constant. */
		inline void SetGravity(_In_ Vector3 value)
		{
			g = value;
		}

		/* Gets the gravitational constant. */
		_Check_return_ inline Vector3SSE GetGravity(void) const
		{
			return g;
		}

		/* Updates the coefficient of drag for a specific object. */
		inline void UpdateParameters(_In_ size_t idx, _In_ float CoD, _In_ float imass)
		{
			cod.set(idx, CoD);
			m.set(idx, imass);
		}

		/* Gets whether the specified object is sleeping. */
		_Check_return_ inline bool IsSleeping(_In_ size_t idx) const
		{
			return sleep.get(idx) == 0.0f;
		}

		/* Adds a specific linear and angular force to the specific object. */
		void AddForce(_In_ size_t idx, _In_ float x, _In_ float y, _In_ float z, _In_ Vector3 torque);
		/* Adds a single kinematic item to the movement system, return the index. */
		_Check_return_ size_t AddItem(_In_ Vector3 p, _In_ Vector3 v, _In_ Quaternion theta, _In_ Vector3 omega, _In_ float CoD, _In_ float imass, _In_ const Matrix3 &moi);
		/* Adds a single static item to the movement system, returns the index. */
		_Check_return_ size_t AddItem(_In_ const Matrix &transform);
		/* Removes the item at the specified index. */
		void RemoveItem(_In_ PhysicsHandle handle);
		/* Adds the gravitational force to the objects. */
		void ApplyGravity(_In_ ofloat dt);
		/* Adds the aerodynamic drag force to the objects. */
		void ApplyDrag(_In_ ofloat dt);
		/* Adds the linear and angular velocity to the objects position. */
		void Integrate(_In_ ofloat dt);
		/* Creates a transformation matrix for the specified object. */
		_Check_return_ Matrix GetTransform(_In_ PhysicsHandle handle) const;
		/* Returns the velocity of the specified object. */
		_Check_return_ Vector3 GetVelocity(_In_ size_t idx) const;
		/* Returns the indices of the objects that have moved out of their expanded AABB. */
		void CheckDistance(_Out_ vector<std::pair<size_t, Vector3>> &result) const;
		/* Sets the sleep bit for any object with a velocity magnitude smaller than the specified epsilon. */
		void TrySleep(_In_ ofloat epsilon);

#ifdef _DEBUG
		/* Visualizes the forces being applied to kinematic objects. */
		void Visualize(_In_ DebugRenderer &dbgRenderer, _In_ float dt) const;
#endif

	private:
		Vector3SSE g;

#ifdef _DEBUG
		struct TimedForce
		{
			float TTL;
			float Magnitude;
			Vector3 Position;
			Vector3 Direction;
		};

		mutable bool addForces;
		mutable vector<TimedForce> forces;
#endif

		avxf_vector cod;
		avxf_vector m;
		avxf_vector m00;
		avxf_vector m01;
		avxf_vector m02;
		avxf_vector m10;
		avxf_vector m11;
		avxf_vector m12;
		avxf_vector m20;
		avxf_vector m21;
		avxf_vector m22;

		avxf_vector px;
		avxf_vector py;
		avxf_vector pz;

		vector<Matrix> transforms;
		mutable avxf_vector qx;
		mutable avxf_vector qy;
		mutable avxf_vector qz;

		avxf_vector vx;
		avxf_vector vy;
		avxf_vector vz;
		avxf_vector sleep;

		avxf_vector ti;
		avxf_vector tj;
		avxf_vector tk;
		avxf_vector tr;

		avxf_vector wp;
		avxf_vector wy;
		avxf_vector wr;
	};
}