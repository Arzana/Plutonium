#include "Environment/Terrain/Plate.h"
#include "Core/Diagnostics/Logging.h"

#define pumalloc(count, type)	reinterpret_cast<type*>(malloc(count * sizeof(type)))
#define pucalloc(count, type)	reinterpret_cast<type*>(calloc(count, sizeof(type)))

Pu::Plate::Plate(const float * map, size_t w, size_t h, size_t _x, size_t _y, size_t age, size_t stride, std::default_random_engine & rng, const TectonicSettings & settings)
	: size(w, h), stride(stride), mass(0.0f), vloc(settings.InitialSpeed), position(_x, _y), rng(&rng), dist01f(0.0f, 1.0f), dist01i(0, 1), settings(&settings)
{
	/* Calculate the (rectangular) area of the plate and allocate the required buffers. */
	const size_t area = w * h;
	heightMap = pumalloc(area, float);
	ageMap = pumalloc(area, size_t);
	ids = pumalloc(area, size_t);
	memset(ids, 255, area * sizeof(size_t));

	/* Set the velocity to a random direction. */
	dir = Vector2::FromAngle(TAU * dist01f(rng)) * settings.InitialSpeed;
	rads = -dist01i(rng) * PI * 0.01f * dist01f(rng);

	for (size_t y = 0, i = 0; y < h; y++)
	{
		for (size_t x = 0; x < w; x++, i++)
		{
			const float v = map[i];

			/* Add the height of the map at the specified position as mass and update the center of mass. */
			mass += heightMap[i] = v;
			com += Vector2(x * v, y * v);

			/* Set the age of this plate to a constant value based on it's height. */
			ageMap[i] = age & -(v > 0.0f);
		}
	}

	/* Normalize the center of mass. */
	com /= mass;
}

Pu::Plate::Plate(Plate && value)
	: heightMap(value.heightMap), ageMap(value.ageMap), ids(value.ids), size(value.size), stride(value.stride),
	dir(value.dir), mass(value.mass), position(value.position), com(value.com), vloc(value.vloc), settings(value.settings),
	accel(value.accel), rads(value.rads), segments(std::move(value.segments)), active(value.active), rng(value.rng)
{
	value.heightMap = nullptr;
	value.ageMap = nullptr;
	value.ids = nullptr;
}

Pu::Plate & Pu::Plate::operator=(Plate && other)
{
	if (this != &other)
	{
		Destroy();

		heightMap = other.heightMap;
		ageMap = other.ageMap;
		ids = other.ids;
		size = other.size;
		stride = other.stride;
		mass = other.mass;
		position = other.position;
		com = other.com;
		vloc = other.vloc;
		dir = other.dir;
		accel = other.accel;
		rads = other.rads;
		segments = std::move(other.segments);
		active = other.active;
		rng = other.rng;
		settings = other.settings;

		other.heightMap = nullptr;
		other.ageMap = nullptr;
		other.ids = nullptr;
	}

	return *this;
}

size_t Pu::Plate::AddCollision(LSize pos)
{
	const size_t id = ids[GetMapIndex(pos)];
	Segment &segment = id >= segments.size() ? CreateSegment(pos) : segments[id];

	++segment.count;
	return segment.area;
}

void Pu::Plate::AddCrustSubduction(const PlateCollisionInfo & info, size_t time)
{
	/* Make sure that the acceleration is changed based on the direction of both plates. */
	const float d = dot(dir, info.GetSecond().GetVelocity());
	accel -= dir * (d > 0.0f);

	/* Gets the position of the subduction (with a bit of randomness put in). */
	float offset = dist01f(*rng);
	offset *= sqr(offset) * (2.0f * dist01i(*rng) - 1.0f);
	LSize pos = info.GetPosition() + Vector2(10.0f * info.GetSecond().GetVelocity() + 3.0f * offset).Truncate();

	if (size.X == stride) pos.X &= size.X - 1;
	if (size.Y == stride) pos.Y &= size.Y - 1;

	/* Update the height and age map at the specified location. */
	const size_t i = pos.Y * size.X + pos.X;
	if (i < size.X * size.Y && heightMap[i] > 0.0f)
	{
		time = (ageMap[i] + time) >> 1;
		ageMap[i] = time * (info.GetCrush() > 0.0f);

		heightMap[i] += info.GetCrush();
		mass += info.GetCrush();
	}
}

void Pu::Plate::ApplyFriction(float deformingMass)
{
	/*
	Remove the energy that deformation consumes from the plate's kinetic energy.
	F - dF = ma - dF => a = dF / m
	*/
	if (mass > 0.0f)
	{
		float velDec = settings->DeformationWeight * deformingMass / mass;
		velDec = velDec < vloc ? velDec : vloc;

		vloc -= velDec;
	}
}

void Pu::Plate::Collide(const PlateCollisionInfo & info)
{
	/* Calculate the colliding mass from the second plate. */
	const float amount = info.GetSecond().AggregateCrust(*this, info.GetPosition());

	/* Compute the normal of the collision based on the positions and the centers of mass. */
	const Vector2 pos(info.GetPosition());
	const Vector2 n = normalize(Vector2((pos - com) - (pos - info.GetSecond().com)));

	/* Early out if the objects are moving away from each other. */
	const Vector2 rdir = dir - info.GetSecond().dir;
	const float d = dot(rdir, n);
	if (d <= 0.0f) return;

	/*
	Calculate the force of impulse.
	Coefficient of restitution is zero for a rigid collision.
	-(1 + CoR) * d / (n . n * (1 / m_1 + 1 / m_2))
	*/
	const float imass = 1.0f / mass + 1.0f / amount;
	const float demon = n.LengthSquared() * imass;
	const float impulse = -1.0f * d / demon;

	/* Calculate the change in trajectory. */
	accel += n * impulse / mass;
	info.GetSecond().accel -= n * impulse / (amount + mass);
}

void Pu::Plate::Erode(void)
{
	/* Reset the mass and center of mass. */
	mass = 0.0f;
	com = Vector2();

	/* Allocate a new target buffer for the height. */
	float *tmp = reinterpret_cast<float*>(calloc(size.X * size.Y, sizeof(float)));
	for (size_t y = 0; y < size.Y; y++)
	{
		for (size_t x = 0, i = 0; x < size.X; x++, i++)
		{
			/* Set the new height to the old height as a default. */
			const float oldHeight = heightMap[i];
			tmp[i] = oldHeight;

			/* Update the plates mass and center of mass. */
			mass += oldHeight;
			com += Vector2(x * oldHeight, y * oldHeight);

			/* Skip the actual erosion if the height at this point is already at the lowest point. */
			if (oldHeight < settings->ContinentBase) continue;

			/* Calculate the indices in the map for the cardinal directions from this point. */
			const size_t maskW = -((x > 0) | (size.X == stride));
			const size_t maskE = -((x < size.X - 1) | (size.X == stride));
			const size_t maskN = -((y > 0) | (size.Y == stride));
			const size_t maskS = -((y < size.Y - 1) | (size.Y == stride));
			size_t w = (stride + x - 1) & (stride - 1) & maskW;
			size_t e = (stride + x + 1) & (stride - 1) & maskE;
			size_t n = (stride + y - 1) & (stride - 1) & maskN;
			size_t s = (stride + y + 1) & (stride - 1) & maskS;
			w = y * size.X + w;
			e = y * size.X + e;
			n = n * size.X + x;
			s = s * size.X + x;

			/* Get the height of the neighbors in the cardinal directions. */
			const float crustW = heightMap[w] * (maskW & (heightMap[w] < oldHeight));
			const float crustE = heightMap[e] * (maskE & (heightMap[e] < oldHeight));
			const float crustN = heightMap[n] * (maskN & (heightMap[n] < oldHeight));
			const float crustS = heightMap[s] * (maskS & (heightMap[s] < oldHeight));

			/* Skip erosion if we're already at the lowest point for all neighbors. */
			if (crustW + crustE + crustN + crustS == 0.0f) continue;

			/* Calculate the difference in height between this point and its neighbors that are lower than this point. */
			const float diffW = oldHeight - crustW;
			const float diffE = oldHeight - crustE;
			const float diffN = oldHeight - crustN;
			const float diffS = oldHeight - crustS;

			float diffMin = diffW;
			diffMin -= (diffMin - diffE) * (diffE < diffMin);
			diffMin -= (diffMin - diffN) * (diffN < diffMin);
			diffMin -= (diffMin - diffS) * (diffS < diffMin);

			/* Calculate the sum of differences between lower neighbors and the tallest lower neightbor. */
			const float diffSum =
				(diffW - diffMin) * (crustW > 0.0f) +
				(diffE - diffMin) * (crustE > 0.0f) +
				(diffN - diffMin) * (crustN > 0.0f) +
				(diffS - diffMin) * (crustS > 0.0f);

#ifdef _DEBUG
			if (diffSum < 0.0f) Log::Fatal("Erosion differense sum is less that zero (%f > %f %f %f %f)!", diffMin, diffW, diffE, diffN, diffS);
#endif

			if (diffSum < diffMin)
			{
				/* There is not enough room in the neighbors to contain all the crust from this peak. */
				tmp[w] += (diffW - diffMin) * (crustW > 0);
				tmp[e] += (diffE - diffMin) * (crustE > 0);
				tmp[n] += (diffN - diffMin) * (crustN > 0);
				tmp[s] += (diffS - diffMin) * (crustS > 0);
				tmp[i] -= diffMin;

				diffMin -= diffSum;

				/* Spread the remaining crust equally among all lower neighbors. */
				diffMin /= 1.0f + (crustW > 0.0f) + (crustE > 0.0f) +
					(crustN > 0.0f) + (crustS > 0.0f);

				tmp[w] += diffMin * (crustW > 0);
				tmp[e] += diffMin * (crustE > 0);
				tmp[n] += diffMin * (crustN > 0);
				tmp[s] += diffMin * (crustS > 0);
				tmp[i] += diffMin;
			}
			else
			{
				/* Remove all crust from the current location; making it as tall as the tallest lower neighbor. */
				float unit = diffMin / diffSum;
				tmp[i] -= diffMin;

				/* Spread all removed crust among all other lower neighbors. */
				tmp[w] += unit * (diffW - diffMin) * (crustW > 0.0f);
				tmp[e] += unit * (diffE - diffMin) * (crustE > 0.0f);
				tmp[n] += unit * (diffN - diffMin) * (crustN > 0.0f);
				tmp[s] += unit * (diffS - diffMin) * (crustS > 0.0f);
			}
		}
	}

	/* Swap the heightmaps and normalize the center of mass. */
	free(heightMap);
	heightMap = tmp;
	com /= mass;
}

void Pu::Plate::GetCollisionStats(const PlateCollisionInfo & info, size_t & count, float & ratio) const
{
	const size_t id = ids[GetMapIndex(info.GetPosition())];

	/* Get the collision count and area for the specified collision; 1 is added to prevent division by zero. */
	const Segment& segment = segments[id];
	count = segment.count;
	ratio = static_cast<float>(count) / static_cast<float>(1 + segment.area);
}

void Pu::Plate::Move(void)
{
	/* Update the velocity and reset the acceleration. */
	dir += accel;
	accel = Vector2();

	/* Split the velocity vector into a unit direction and a velocity scalar. */
	const float len = dir.Length();
	dir /= len;
	vloc = max(0.0f, vloc + (len - 1.0f));

	/* Apply rotational impulses to the plate. */
	dir.Rotate(rads * sqr(vloc));

	/* Add the final velocity to the position and modulate it between [0, stride]. */
	position += (dir * vloc).Truncate();
	position.X = rclamp(position.X, 0LL, static_cast<int64>(stride));
	position.Y = rclamp(position.Y, 0LL, static_cast<int64>(stride));
}

void Pu::Plate::SetCrust(LSize pos, float amnt, size_t time)
{
	const size_t w = size.X;
	const size_t h = size.Y;
	size_t area = w * h;

	/* Crust should only be possitive. */
	amnt = max(0.0f, amnt);

	/* Check if new crust has to be made. */
	size_t i = GetMapIndex(pos);
	if (i >= area)
	{
		/* Make sure that the position cannot be outside of the map bounds. */
		position.X &= stride;
		position.Y &= stride;

		/* Calculate the distance of the new point from the plates edges. */
		const size_t left = position.X - pos.X;
		const size_t right = (stride & -(static_cast<int64>(pos.X) < position.X)) + pos.X - (position.X + w - 1);
		const size_t top = position.Y - pos.Y;
		const size_t bottom = (stride & -(static_cast<int64>(pos.Y) < position.Y)) + pos.Y - (position.Y + h - 1);

		/* Set the largest horizontal/vertical distance to zero, and clamp it the the world stride. */
		size_t dleft = left & -(left < right) & -(left < stride);
		size_t dright = right & (right <= left) & -(right < stride);
		size_t dtop = top & -(top < bottom) & -(top < stride);
		size_t dbottom = bottom & -(bottom <= top) & -(bottom < stride);

		/* Scale by 8. */
		dleft = ((dleft > 0) + (dleft >> 3)) << 3;
		dright = ((dright > 0) + (dright >> 3)) << 3;
		dtop = ((dtop > 0) + (dtop >> 3)) << 3;
		dbottom = ((dbottom > 0) + (dbottom >> 3)) << 3;

		/* Make sure that the plate doesn't grow bigger than the system it's in. */
		if (w + dleft + dright > stride)
		{
			dleft = 0;
			dright = stride - w;
		}

		if (h + dtop + dbottom > stride)
		{
			dtop = 0;
			dbottom = stride - h;
		}

		/* Update the parameters. */
		position.X -= dleft;
		position.X += position.X >= 0 ? 0 : stride;
		size.X += dleft + dright;

		position.Y -= dtop;
		position.Y += top >= 0 ? 0 : stride;
		size.Y += dtop + dbottom;

		/*
		Create the new buffers for the plate.
		Not allocating memory here would speed up performance a lot!
		*/
		area = size.X * size.Y;
		float *tmpH = pucalloc(area, float);
		size_t *tmpA = pucalloc(area, size_t);
		size_t *tmpI = pumalloc(area, size_t);
		memset(tmpI, 255, area * sizeof(size_t));

		/* Copy the old plate into the new one. */
		for (size_t y = 0, j, k; y < h; y++)
		{
			j = y * w;
			k = (dtop + y) * size.X + dleft;

			memcpy(tmpH + k, heightMap + j, w * sizeof(float));
			memcpy(tmpA + k, ageMap + j, w * sizeof(size_t));
			memcpy(tmpI + k, ids + j, w * sizeof(size_t));
		}

		/* Swap the old data for the new data. */
		free(heightMap);
		free(ageMap);
		free(ids);
		heightMap = tmpH;
		ageMap = tmpA;
		ids = tmpI;

		/* Shift the segment data over to match the new coordinates. */
		const LSize offset(dleft, dtop);
		for (Segment &segment : segments)
		{
			segment.lower += offset;
			segment.upper += offset;
		}

		/* Get the new map index for the shifted plate. */
		i = GetMapIndex(pos);
	}

	/* Update the crust's age. */
	const size_t oldCrust = -(heightMap[i] > 0.0f);
	const size_t newCrust = -(amnt > 0.0f);
	time = (time & ~oldCrust) | ((ageMap[i] + time) / 2 & oldCrust);

	/* Update the height, age and mass of the plate. */
	heightMap[i] = amnt;
	ageMap[i] = (time & newCrust) | (ageMap[i] & newCrust);
	mass = mass - heightMap[i] + amnt;
}

void Pu::Plate::AddCrustCollision(LSize pos, float amnt, size_t time)
{
	/* Add the crust and extend the plate if needed. */
	SetCrust(pos, GetCrust(pos) + amnt, time);

	const size_t i = GetMapIndex(pos);
	ids[i] = active;

	Segment &segment = segments[active];
	++segment.area;

	segment.lower.X = min(segment.lower.X, pos.X);
	segment.lower.Y = min(segment.lower.Y, pos.Y);
	segment.upper.X = max(segment.upper.X, pos.X);
	segment.upper.Y = max(segment.upper.Y, pos.Y);
}

float Pu::Plate::AggregateCrust(Plate & other, LSize pos)
{
	/* Get the segment associated with the crust. */
	const size_t id = ids[GetMapIndex(pos)];
	Segment &segment = segments[id];

	/*
	Early out if this is a second order collision.
	One continent can have many points of collision.
	If one of them causes the continent to aggregate then all later collisions
	and attemopts of aggregation would change nothing at all, because the continent was removed from the plate.
	*/
	if (segment.area == 0) return 0.0f;

	/* Mark the segment as active. */
	active = id;
	pos += LSize(stride);
	const float oldMass = mass;

	/* Add all of the collided continent's crust to the destination plate. */
	for (size_t y = segment.lower.Y, i = 0; y < segment.upper.Y; y++)
	{
		for (size_t x = segment.lower.X; x < segment.upper.X; x++, i++)
		{
			/* Only add crust if needed. */
			if ((ids[i] == id) && (heightMap[i] > 0.0f))
			{
				other.AddCrustCollision(pos + LSize(x, y), heightMap[i], ageMap[i]);
				mass -= heightMap[i];
				heightMap[i] = 0.0f;
			}
		}
	}

	/* Mark the segment as non-existent and return the difference in mass. */
	segment.area = 0;
	return oldMass - mass;
}

size_t Pu::Plate::GetCrustAge(LSize pos) const
{
	const size_t i = GetMapIndex(pos);
	return i < maxv<size_t>() ? ageMap[i] : 0;
}

float Pu::Plate::GetCrust(LSize pos) const
{
	const size_t i = GetMapIndex(pos);
	return i < maxv<size_t>() ? heightMap[i] : 0.0f;
}

Pu::Plate::Segment & Pu::Plate::CreateSegment(LSize pos)
{
	/* Get the index of the specified position and the maximum segment id. */
	const size_t i = pos.Y * size.X + pos.X;
	const size_t maxID = segments.size();

	const bool left = pos.X > 0 && heightMap[i - 1] >= settings->ContinentBase;
	const bool right = pos.X < size.X && heightMap[i + 1] >= settings->ContinentBase;
	const bool top = pos.Y > 0 && heightMap[i - size.X] >= settings->ContinentBase;
	const bool bottom = pos.Y < size.Y - 1 && heightMap[i + size.X] >= settings->ContinentBase;

	/*
	The current point is not yet part of a segment.
	It might be next to an existing segment, if so associate this point with it.
	*/
	size_t neightborId = maxID;
	if (left && ids[i - 1] < maxID) neightborId = ids[i - 1];
	else if (right && ids[i + 1] < maxID) neightborId = ids[i + 1];
	else if (top && ids[i - size.X] < maxID) neightborId = ids[i - size.X];
	else if (bottom && ids[i + size.X] < maxID) neightborId = ids[i + size.X];

	/*
	Early out if we can associate the point with an existing segment.
	Mainly used for oceanic crust.
	*/
	if (neightborId != maxID)
	{
		ids[i] = neightborId;
		Segment &segment = segments[neightborId];

		/* Increase the segments area. */
		++segment.area;
		segment.lower.X = min(segment.lower.X, pos.X);
		segment.lower.Y = min(segment.lower.Y, pos.Y);
		segment.upper.X = max(segment.upper.X, pos.X);
		segment.upper.Y = max(segment.upper.Y, pos.Y);

		return segment;
	}

	Segment segment{ pos, pos, 0, 0 };
	vector<vector<size_t>> spansToDo;
	vector<vector<size_t>> spansDone;

	spansToDo.resize(size.Y);
	spansDone.resize(size.Y);

	ids[i] = maxID;
	spansToDo[pos.Y].emplace_back(pos.X);
	spansToDo[pos.Y].emplace_back(pos.X);

	size_t lines;
	do
	{
		lines = 0;

		for (size_t y = 0, start, end; y < size.Y; y++)
		{
			vector<size_t> &lineToDo = spansToDo[y];
			vector<size_t> &lineDone = spansDone[y];

			/* Skip this line if its todo list is empty. */
			if (lineToDo.empty()) continue;

			/* Find an unscanned span on this line. */
			do
			{
				end = lineToDo.back();
				lineToDo.pop_back();

				start = lineToDo.back();
				lineToDo.pop_back();

				/* Remove any done spans from this span. */
				for (size_t x = 0; x < lineDone.size(); x += 2)
				{
					if (start >= lineDone[x] && start <= lineDone[x + 1]) start = lineDone[x + 1] + 1;
					if (end >= lineDone[x] && end <= lineDone[x + 1]) end = lineDone[x] - 1;
				}

				start |= -(end >= size.X);
				end -= (end >= size.X);

			} while (start > end && !lineToDo.empty());

			/* Skip if there is nothing left to do. */
			if (start > end) continue;

			/* Calculate the line indeces. */
			const size_t line = y * size.X;
			const size_t rowAbove = ((y - 1) & -(y > 0)) | ((size.Y - 1) & -(y == 0));
			const size_t rowBelow = (y + 1) & -(y < size.Y - 1);
			const size_t lineAbove = rowAbove * size.X;
			const size_t lineBelow = rowBelow * size.X;

			/* Extend the beginning of the line. */
			while (start > 0 && ids[line + start - 1] > maxID && heightMap[line + start - 1] >= settings->ContinentBase)
			{
				--start;
				ids[line + start] = maxID;
			}

			/* Extend the end of the line. */
			while (end < size.X - 1 && ids[line + end + 1] > maxID && heightMap[line + end + 1] >= settings->ContinentBase)
			{
				++end;
				ids[line + end] = maxID;
			}

			if (size.X == stride)
			{
				/* Check if it should wrap around the left edge. */
				if (start == 0 && ids[line + size.X - 1] > maxID && heightMap[line + size.X - 1] >= settings->ContinentBase)
				{
					ids[line + size.X + 1] = maxID;
					spansToDo[y].emplace_back(size.X - 1);
					spansToDo[y].emplace_back(size.X - 1);
				}

				/* Check if it should wrap around the right edge. */
				if (end == size.X - 1 && ids[line] > maxID && heightMap[line] >= settings->ContinentBase)
				{
					ids[line] = maxID;
					spansToDo[y].emplace_back(0);
					spansToDo[y].emplace_back(0);
				}
			}

			/* Update the segment's area, should be at least one per iteration. */
			segment.area += 1 + end - start;
			segment.lower.X = min(segment.lower.X, start);
			segment.lower.Y = min(segment.lower.Y, y);
			segment.upper.X = max(segment.upper.X, end);
			segment.upper.Y = max(segment.upper.Y, y);

			if (y > 0 || size.Y == stride)
			{
				for (size_t x = start; x < end; x++)
				{
					size_t &id = ids[lineAbove + x];
					if (id > maxID && heightMap[lineAbove + x] >= settings->ContinentBase)
					{
						id = maxID;
						spansToDo[rowAbove].emplace_back(x);
						spansToDo[rowAbove].emplace_back(x);
						x++;
					}
				}
			}

			if (y < size.Y - 1 || size.Y == stride)
			{
				for (size_t x = start; x < end; x++)
				{
					size_t &id = ids[lineBelow + x];
					if (id > maxID && heightMap[lineBelow + x] >= settings->ContinentBase)
					{
						id = maxID;
						spansToDo[rowBelow].emplace_back(x);
						spansToDo[rowBelow].emplace_back(x);
						x++;
					}
				}
			}

			/* Mark the current line as done. */
			spansDone[y].emplace_back(start);
			spansDone[y].emplace_back(end);
			lines++;
		}
	} while (lines > 0);

	/* Add the new segment to the list and return it. */
	return segments.emplace_back(segment);
}

size_t Pu::Plate::GetMapIndex(LSize pos) const
{
	const size_t mask = stride - 1;
	return (pos.Y & mask) * size.X + (pos.X & mask);
}

void Pu::Plate::Destroy(void)
{
	if (heightMap) free(heightMap);
	if (ageMap) free(ageMap);
	if (ids) free(ids);
}