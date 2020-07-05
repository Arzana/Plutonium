#pragma once
#include "Physics/Objects/PhysicsHandle.h"
#include "Core/Collections/fvector.h"
#include "Core/Math/VectorSSE.h"
#include "Core/Math/Matrix.h"

namespace Pu
{
	/* Defines a system that handles the integration of position. */
	class MovementSystem
	{
	public:
		/* Initializes a new instance of a movement system. */
		MovementSystem(void);
		/* Copy constructor. */
		MovementSystem(_In_ const MovementSystem &value) = default;
		/* Move constructor. */
		MovementSystem(_In_ MovementSystem &&value) = default;

		/* Copy assignment. */
		_Check_return_ MovementSystem& operator =(_In_ const MovementSystem &other) = default;
		/* Move assignment. */
		_Check_return_ MovementSystem& operator =(_In_ MovementSystem &&other) = default;

		/* Sets the gravitational constant. */
		inline void SetGravity(_In_ Vector3 value)
		{
			g = value;
		}

		/* Updates the coefficient of drag for a specific object. */
		inline void UpdateParameters(_In_ size_t idx, _In_ float CoD, _In_ float imass)
		{
			cod.set(idx, CoD);
			m.set(idx, imass);
		}

		/* Adds a specific linear and angular force to the specific object. */
		void AddForce(_In_ size_t idx, _In_ float x, _In_ float y, _In_ float z, _In_ Quaternion angular);
		/* Adds a single kinematic item to the movement system, return the index. */
		_Check_return_ size_t AddItem(_In_ Vector3 p, _In_ Vector3 v, _In_ Quaternion theta, _In_ Quaternion omega, _In_ float CoD, _In_ float imass);
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

	private:
		Vector3SSE g;

		fvector cod;
		fvector m;

		fvector px;
		fvector py;
		fvector pz;

		vector<Matrix> transforms;
		mutable fvector qx;
		mutable fvector qy;
		mutable fvector qz;

		fvector vx;
		fvector vy;
		fvector vz;

		fvector ti;
		fvector tj;
		fvector tk;
		fvector tr;

		fvector wi;
		fvector wj;
		fvector wk;
		fvector wr;
	};
}