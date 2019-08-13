#pragma once
#include <random>
#include "TectonicSettings.h"
#include "PlateCollisionInfo.h"
#include "Core/Math/Vector2.h"

namespace Pu
{
	/* 
	Defines a tectonic plate.
	https://www.theseus.fi/bitstream/handle/10024/40422/Viitanen_Lauri_2012_03_30.pdf
	*/
	class Plate
	{
	public:
		Plate(_In_ const Plate&) = delete;
		/* Move constructor. */
		Plate(_In_ Plate &&value);
		/* Releases the resources allocated by the plate. */
		~Plate(void)
		{
			Destroy();
		}

		_Check_return_ Plate& operator =(_In_ const Plate&) = delete;
		/* Move assignment. */
		_Check_return_ Plate& operator =(_In_ Plate &&other);

		/* Increments the collision counter of the continent at a given location, returns the surface area of the collided continent. */
		_Check_return_ size_t AddCollision(_In_ LSize pos);
		/* Adds a specified amount of crust to the plate as a result of a subducting oceanic plate under this plate at a specific time. */
		void AddCrustSubduction(_In_ const PlateCollisionInfo &info, _In_ size_t time);
		/* Decreases the speed of the plate. */
		void ApplyFriction(_In_ float deformingMass);
		/* Handles the collision between tro plates at the specified world position. */
		void Collide(_In_ const PlateCollisionInfo &info);
		/* Applies plate wide erosion */
		void Erode(void);
		/* Gets the collision statistics of the continent at the collision location. */
		void GetCollisionStats(_In_ const PlateCollisionInfo &info, _Out_ size_t &count, _Out_ float &ratio) const;
		/* Gets the timestamp of the plate's crustal material at the specified location. */
		_Check_return_ size_t GetCrustAge(_In_ LSize pos) const;
		/* Move the plate along its trajectory. */
		void Move(void);
		/* Sets the amount of crustal meterial at the specified location. */
		void SetCrust(_In_ LSize pos, _In_ float amnt, _In_ size_t time);

		/* Clears earlier continental crust partitions. */
		inline void ResetSegments(void)
		{
			segments.clear();
		}

		/* Gets the height of the plate at a specific index. */
		_Check_return_ inline float GetHeight(_In_ size_t idx) const
		{
			return heightMap[idx];
		}

		/* Gets the age of the plate at a specific index. */
		_Check_return_ inline size_t GetAge(_In_ size_t idx) const
		{
			return ageMap[idx];
		}

		/* Gets the current momentum of the plate. */
		_Check_return_ inline float GetMomentum(void) const
		{
			return mass * vloc;
		}

		/* Gets the size of this plate. */
		_Check_return_ inline LSize GetSize(void) const
		{
			return size;
		}

		/* Gets the location of this plate. */
		_Check_return_ inline LPoint GetLocation(void) const
		{
			return position;
		}

		/* Gets the velocity of this plate. */
		_Check_return_ inline float GetVelocity(void) const
		{
			return vloc;
		}

	private:
		friend class Lithosphere;

		struct Segment
		{
			LSize lower, upper;
			size_t area, count;
		};

		float *heightMap;
		size_t *ageMap;
		size_t *ids;

		vector<Segment> segments;
		size_t active;

		LPoint position;
		LSize size;
		size_t stride;

		float mass;
		float vloc;
		Vector2 com;
		Vector2 dir;
		Vector2 accel;
		float rads;

		const TectonicSettings *settings;
		std::default_random_engine *rng;
		std::uniform_real_distribution<float> dist01f;
		std::uniform_int_distribution<int> dist01i;

		Plate(const float *map, size_t w, size_t h, size_t x, size_t y, size_t age, size_t stride, std::default_random_engine &rng, const TectonicSettings &settings);

		void AddCrustCollision(LSize pos, float amnt, size_t time);
		float AggregateCrust(Plate &other, LSize pos);
		float GetCrust(LSize pos) const;
		Segment& CreateSegment(LSize pos);
		size_t GetMapIndex(LSize pos) const;
		void Destroy(void);
	};
}