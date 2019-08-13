#pragma once
#include "Core/Math/Constants.h"

namespace Pu
{
	/* Defines the settings used for the tectonic plates simulation. */
	struct TectonicSettings
	{
	public:
		/* Defines the seed to be used for all random numbers. */
		uint64 Seed;
		/* Defines the initial speed of the plates (default is 10). */
		float InitialSpeed;
		/* Defines the weight of deformations on the velocity of a plate on impact (default is 5). */
		float DeformationWeight;
		/* 
		Defines the height that seperates the sea from dry land (default is 1). 
		This is also the initial value for continental crust on the heightmap.
		*/
		float ContinentBase;
		/* 
		Defines the height of the lowest point of a sea (default is 0.1).
		This is also the initial value for oceanic crust on the heightmap.
		*/
		float OceanicBase;
		/* 
		Defines the amount of surface area that becomes oceanic crust (default is 0.65).
		Zero means only oceanic crust.
		One means only continental crust.
		*/
		float SeaLevel;
		/* Defines the percentage of overlapping crust that is folded (default is 0.001). */
		float FoldingRatio;
		/* Defines the percentage of overlapping area causing aggregation (default is 0.1). */
		float AggregationRatio;
		/* 
		Defines the initial roughness of the topography before any lithosphere update (default is 0.5).
		This is used as a weight for a square-diamond noise algorithm and should be in the range [0, 1].
		*/
		float InitialRoughness;
		/* Defines the buoyancy modifier for new oceanic crust (default is 3). */
		float BuoyancyBoost;
		/* Defines the minimum energy required in the lithosphere to allow it to continue (default is 0.15). */
		float RestartEnergyRatio;
		/* Defines the minimum amount of velocity required in the system to allow it to continue (default is 2). */
		float RestartMinimumSpeed;
		/* Defines the number of iterations between global erosion cycles (deafult is 60). */
		size_t ErosionPeriod;
		/* Defines the number of overlapping points causing aggregation (default is 5000). */
		size_t AggregationAmount;
		/*
		Defines the number of times that the system will be restarted (default is 2).
		A restart occurs if either of the following conditions are met:
		- The total velocity is lower than RestartMinimumSpeed.
		- The ration of kinetic energy / peak kinetic energy is less than RestartEnergyRatio.
		- The last continental collision happened NoCollisionTimeLimit update's ago.
		- The number of updates goes above MaxIterations.
		*/
		size_t Cycles;
		/* Defines the number of plates to be created (default is 10). */
		size_t PlateCount;
		/* Defines the maximum age of new oceanic crust where buoyancy is applied (default is 20). */
		size_t MaxBuoyancyAge;
		/* Defines the maximum amount of time the system can go without collisions before needing to restart (default is 10). */
		size_t NoCollisionTimeLimit;
		/* Defines the amount of updates before the system will restart regardless of energy conditions (default is 600). */
		size_t MaxIterations;
		/*
		Defines whether to create new crust material at divergent boundaries (default is true).
		Disabling this will lead to holes in the topography it's not recomended to disable this.
		*/
		bool RegenerateCrust;

		/* Initializes a default instance of the tectonic settings object. */
		TectonicSettings(void)
			: InitialSpeed(10.0f), DeformationWeight(5.0f), 
			ContinentBase(1.0f), OceanicBase(0.1f), SeaLevel(0.65f),
			AggregationRatio(0.1f), InitialRoughness(0.5f), 
			AggregationAmount(5000), FoldingRatio(0.001f), 
			ErosionPeriod(60), Cycles(2), PlateCount(10), 
			RegenerateCrust(true), BuoyancyBoost(3.0f), Seed(0),
			MaxBuoyancyAge(20), RestartEnergyRatio(0.15f),
			RestartMinimumSpeed(2.0f), NoCollisionTimeLimit(10),
			MaxIterations(600)
		{}
	};
}