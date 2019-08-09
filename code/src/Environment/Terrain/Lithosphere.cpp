#include "Environment/Terrain/Lithosphere.h"
#include "Environment/Terrain/Plate.h"

#define pumalloc(count, type)	reinterpret_cast<type*>(malloc(count * sizeof(type)))
#define pucalloc(count, type)	reinterpret_cast<type*>(calloc(count, sizeof(type)))

Pu::Lithosphere::Lithosphere(uint16 size, TectonicSettings settings)
	: settings(settings), cycleCount(0), iterCount(settings.PlateCount + settings.MaxBuoyancyAge), 
	stride(size), rng(static_cast<uint32>(settings.Seed)), peakEk(0.0f), lastColl(0), noise(settings.Seed)
{
	size_t area = sqr(stride);

	/* Generate the initial height map, this will be scaled to [0, 1] range (we're altering the buffer slightly so we const_cast). */
	noise.SetSize(size);
	noise.SetRoughness(settings.InitialRoughness);
	float *tmp = const_cast<float*>(noise.GenerateNormalized());

	/* Find the actual value in the height map that produces the continent-sea ratio defined by the sea-level setting. */
	float threshold = 0.5f;
	for (float step = 0.5f, iarea = recip(static_cast<float>(area)); step > 0.01f;)
	{
		size_t count = 0;
		for (size_t i = 0; i < area; i++) count += (tmp[i] < threshold);

		step *= 0.5f;
		if (count * iarea < settings.SeaLevel) threshold += step;
		else threshold -= step;
	}

	/* Finalize the heightmap with the correct sea level. */
	settings.SeaLevel = threshold;
	for (size_t i = 0; i < area; i++)
	{
		float &cur = tmp[i];
		cur = (cur > threshold) * (cur + settings.ContinentBase) + (cur <= threshold) * settings.OceanicBase;
	}

	/* Remove 1 from the stride to get a power of two stride. */
	--stride;
	area = sqr(stride);

	/* Allocate the destination for the heightmap. */
	heightMap = pumalloc(area, float);
	idxMap = pucalloc(area, size_t);

	for (size_t i = 0; i < stride; i++)
	{
		memcpy(heightMap + i * stride, tmp + i * (stride + 1), stride * sizeof(float));
	}

	/* Allocate the starting plates. */
	ReserveVectors();
	CreatePlates();
}

Pu::Lithosphere::Lithosphere(Lithosphere && value)
	: heightMap(value.heightMap), idxMap(value.idxMap), plates(std::move(value.plates)),
	settings(std::move(value.settings)), peakEk(value.peakEk), lastColl(value.lastColl),
	cycleCount(value.cycleCount), iterCount(value.iterCount), stride(value.stride),
	collisions(std::move(value.collisions)), subductions(std::move(value.subductions))
{
	value.heightMap = nullptr;
	value.idxMap = nullptr;
}

Pu::Lithosphere & Pu::Lithosphere::operator=(Lithosphere && other)
{
	if (this != &other)
	{
		Destroy();

		heightMap = other.heightMap;
		idxMap = other.idxMap;
		plates = std::move(other.plates);
		settings = std::move(other.settings);
		peakEk = other.peakEk;
		lastColl = other.lastColl;
		cycleCount = other.cycleCount;
		iterCount = other.iterCount;
		stride = other.stride;
		collisions = std::move(other.collisions);
		subductions = std::move(other.subductions);

		other.heightMap = nullptr;
		other.idxMap = nullptr;
	}

	return *this;
}

void Pu::Lithosphere::Update(void)
{
	Restart();
}

void Pu::Lithosphere::CreatePlates(void)
{
	/* 
	Initialize a lookup table for free plate center positions. 
	This is to make sure that two plates will never have the same center.
	*/
	const size_t area = sqr(stride);
	for (size_t i = 0; i < area; i++) idxMap[i] = i;

	struct PlateArea
	{
		vector<size_t> Border;
		size_t Left;
		size_t Right;
		size_t Top;
		size_t Bottom;

		PlateArea(size_t p, size_t x, size_t y)
			: Left(x), Right(x), Top(y), Bottom(y)
		{
			Border.reserve(8);
			Border.emplace_back(p);
		}
	};

	/* Generate the plate centers. */
	vector<PlateArea> areas;
	areas.reserve(settings.PlateCount);
	for (size_t i = 0; i < settings.PlateCount; i++)
	{
		/* Select a random unused plate origin. */
		std::uniform_int_distribution<size_t> dist(0, area - i - 1);
		const size_t p = idxMap[dist(rng)];

		/* Create the base plate. */
		const size_t y = p / stride;
		const size_t x = p - y * stride;
		areas.emplace_back(p, x, y);

		/* Override the used entry with the last entry in the array. */
		idxMap[p] = idxMap[area - i - 1];
	}

	/* Reset the idx map so we can reuse it, but set the spwan points for the plates. */
	memset(idxMap, 255, area * sizeof(size_t));
	for (size_t i = 0; i < settings.PlateCount; i++) idxMap[areas[i].Top * stride + areas[i].Left] = i;

	/* This mask is used as a faster module operation. */
	const size_t mask = stride - 1;

	/* Grow the plates from their origins until the surface is fully populated. */
	size_t maxBorder = 1;
	while (maxBorder)
	{
		maxBorder = 0;

		/* Expand all plates one by one untill no more borders are available. */
		for (size_t i = 0; i < settings.PlateCount; i++)
		{
			PlateArea &cur = areas[i];
			const size_t len = cur.Border.size();
			maxBorder = max(maxBorder, len);

			/* We can skip this plate if it no longer has any borders. */
			if (len == 0) continue;

			/* Pick a random border point. */
			const size_t j = std::uniform_int_distribution<size_t>(0, len - 1)(rng);
			const size_t p = cur.Border[j];

			/* Get the world position of the point. */
			const size_t y = p / stride;
			const size_t x = p - y * stride;

			/* Get the points to the left, right, top and bottom. */
			const size_t left = x > 0 ? x - 1 : stride - 1;
			const size_t right = x < stride - 1 ? x + 1 : 0;
			const size_t top = y > 0 ? y - 1 : stride - 1;
			const size_t bottom = y < stride - 1 ? y + 1 : 0;

			/* Get the index of those neighbors. */
			const size_t n = top * stride + x;
			const size_t s = bottom * stride + x;
			const size_t w = y * stride + left;
			const size_t e = y * stride + right;

			/* Check if the plate index is not set yet. */
			if (idxMap[n] >= settings.PlateCount)
			{
				/* Set the plate index and add the point to the border. */
				idxMap[n] = i;
				cur.Border.emplace_back(n);

				/* Update the bounding box if needed. */
				cur.Top = min(cur.Top, top);
				cur.Bottom = max(cur.Bottom, top);
			}

			/* Repeat previous step for all sides. */
			if (idxMap[s] >= settings.PlateCount)
			{
				idxMap[s] = i;
				cur.Border.emplace_back(s);

				cur.Top = min(cur.Top, bottom);
				cur.Bottom = max(cur.Bottom, bottom);
			}

			if (idxMap[w] >= settings.PlateCount)
			{
				idxMap[w] = i;
				cur.Border.emplace_back(w);

				cur.Left = min(cur.Left, left);
				cur.Right = max(cur.Right, left);
			}

			if (idxMap[e] >= settings.PlateCount)
			{
				idxMap[e] = i;
				cur.Border.emplace_back(e);

				cur.Left = min(cur.Left, right);
				cur.Right = max(cur.Right, right);
			}

			/* Swap the processed point with an unprocessed point. */
			cur.Border[j] = cur.Border.back();
			cur.Border.pop_back();
		}
	}

	vector<size_t> idxCpy{ idxMap, idxMap + area };

	/* Extract and create plates from the initial terrain. */
	for (size_t i = 0; i < settings.PlateCount; i++)
	{
		PlateArea &cur = areas[i];
		const size_t w = 1 + cur.Right - cur.Left;
		const size_t h = 1 + cur.Bottom - cur.Top;

		/* Copy the plate's height data from the global map into the local map. */
		float *map = pumalloc(w * h, float);
		for (size_t y = cur.Top, j = 0; y <= cur.Bottom; y++)
		{
			for (size_t x = cur.Left; x <= cur.Right; x++, j++)
			{
				const size_t k = (y & mask) * stride + (x & mask);
				map[j] = heightMap[k] * (idxMap[k] == i);
			}
		}

		/* Create the plate. */
		plates.emplace_back(new Plate(map, w, h, cur.Left, cur.Top, i, stride, rng, settings));
		free(map);
	}
}

void Pu::Lithosphere::ReserveVectors(void)
{
	/*
	Pre-allocate the vector storage for collisions and subductions.
	Stride * 4 == map's circumference.
	*/
	vector<PlateCollisionInfo> vec;
	vec.reserve(stride << 2);

	collisions.reserve(settings.PlateCount);
	subductions.reserve(settings.PlateCount);

	for (size_t i = 0; i < settings.PlateCount; i++)
	{
		collisions.emplace_back(vec);
		subductions.emplace_back(vec);
	}

	plates.reserve(settings.PlateCount);
}

void Pu::Lithosphere::Restart(void)
{
	/* Increment the cycle count (unless it should run forever) and return if it's reached its max. */
	cycleCount += settings.Cycles > 0;
	if (cycleCount > settings.Cycles) return;

	const size_t area = sqr(stride);
	const size_t mask = stride - 1;
	size_t *ageMap = pumalloc(area, size_t);

	/* 
	Update the heightmap to include all recent changes. 
	The plates cover the entire map so we can just set the height and age without first clearing them.
	*/
	for (const Plate *cur : plates)
	{
		const LSize pos = static_cast<LSize>(cur->GetLocation());
		const LSize size = pos + cur->GetSize();

		/* Copy over the height and age of the plate. */
		for (size_t y = pos.Y, i = 0; y < size.Y; y++)
		{
			for (size_t x = pos.X; x < size.X; x++, i++)
			{
				const size_t j = (y & mask) * stride + (x & mask);

				heightMap[j] += cur->heightMap[i];
				ageMap[j] = cur->ageMap[i];
			}
		}

		/* Delete the old plate. */
		delete cur;
	}

	/* Create new plates if there are cycles left to run. */
	plates.clear();
	if (cycleCount < settings.Cycles + !settings.Cycles)
	{
		free(ageMap);
		CreatePlates();
		return;
	}

	/* If this is the last cycle then finalize the map. */
	Finalize(ageMap);
	free(ageMap);
}

void Pu::Lithosphere::Finalize(const size_t * ageMap)
{
	const size_t area = sqr(stride);

	/* Add some buoyance to all pixels for a visual boost. */
	for (size_t i = 0; i < (settings.BuoyancyBoost > 0.0f) * area; i++)
	{
		size_t age = iterCount - ageMap[i];
		age = settings.MaxBuoyancyAge - age;
		age &= -(age <= settings.MaxBuoyancyAge);

		heightMap[i] += (heightMap[i] < settings.ContinentBase) * settings.BuoyancyBoost *
			settings.OceanicBase * age * (1.0f / settings.MaxBuoyancyAge);
	}

	/* Add some random noise to the map. */
	const float *tmp = noise.GenerateNormalized();
	for (size_t i = 0; i < area; i++)
	{
		float cur = tmp[i];

		/*
		If it's a continent point then we add the random amount to the height.
		If it's an oceanic point then we change it randomly to create a bumpy ocean floor.
		*/
		if (heightMap[i] > settings.ContinentBase) heightMap[i] += cur * 2.0f;
		else heightMap[i] = 0.8f * heightMap[i] + 0.2f * cur * settings.ContinentBase;
	}
}

void Pu::Lithosphere::Destroy(void)
{
	if (heightMap) delete heightMap;
	if (idxMap) delete idxMap;

	for (const Plate *cur : plates) delete cur;
	plates.clear();
}