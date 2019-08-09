#pragma once
#include "TectonicSettings.h"
#include "PlateCollisionInfo.h"
#include "Core/Collections/vector.h"
#include "Core/Math/SquareDiamondNoise.h"

namespace Pu
{
	class Plate;

	/* Defines the rigid outermost shell of a rocky planet. */
	class Lithosphere
	{
	public:
		/* Initializes a new instance of a lithosphere with a specific size with the specified settings. */
		Lithosphere(_In_ uint16 size, _In_ TectonicSettings settings);
		Lithosphere(_In_ const Lithosphere&) = delete;
		/* Move constructor. */
		Lithosphere(_In_ Lithosphere &&value);
		/* Releases the resources allocated by the lithosphere. */
		~Lithosphere(void)
		{
			Destroy();
		}

		_Check_return_ Lithosphere& operator =(_In_ const Lithosphere&) = delete;
		/* Move assignment. */
		_Check_return_ Lithosphere& operator =(_In_ Lithosphere &&other);

		/* Simulates one step of plate tectonics. */
		void Update(void);

		/* Gets the amount of times the system has been restarted. */
		_Check_return_ inline size_t GetCycleCount(void) const
		{
			return cycleCount;
		}

		/* Gets the iteration count. */
		_Check_return_ inline size_t GetIterationCount(void) const
		{
			return iterCount;
		}

		/* Gets the number of plates active in the lithosphere. */
		_Check_return_ size_t GetPlateCount(void) const
		{
			return plates.size();
		}

		/* Gets the side length of the map. */
		_Check_return_ size_t GetSide(void) const
		{
			return stride;
		}

		/* Gets the size of the map. */
		_Check_return_ size_t GetSize(void) const
		{
			return stride * stride;
		}

		/* Gets the height map of the terrain. */
		_Check_return_ inline const float* GetTopography(void) const
		{
			return heightMap;
		}

		/* Gets the ID's of the owning plate of every point on the terrain. */
		_Check_return_ inline const size_t* GetPlateIDs(void) const
		{
			return idxMap;
		}

	private:
		float *heightMap;
		size_t *idxMap;
		vector<Plate*> plates;

		TectonicSettings settings;
		SquareDiamondNoise noise;

		float peakEk;
		size_t lastColl;
		size_t cycleCount;
		size_t iterCount;
		size_t stride;

		std::default_random_engine rng;
		vector<vector<PlateCollisionInfo>> collisions;
		vector<vector<PlateCollisionInfo>> subductions;

		void CreatePlates(void);
		void ReserveVectors(void);
		void Restart(void);
		void Finalize(const size_t *ageMap);
		void Destroy(void);
	};
}