#include "Physics/Systems/SAT.h"
#include "Physics/Systems/ShapeTests.h"

static Pu::uint32 calls = 0;

Pu::uint32 Pu::SAT::GetCallCount(void)
{
	return calls;
}

void Pu::SAT::ResetCounter(void)
{
	calls = 0;
}

bool Pu::SAT::Run(const AABB & aabb, const OBB & obb)
{
	FillBuffer(c1, axes, aabb);
	FillBuffer(c2, axes + 3, obb);
	return RunInternal();
}

bool Pu::SAT::Run(const OBB & obb1, const OBB & obb2)
{
	FillBuffer(c1, axes, obb1);
	FillBuffer(c2, axes + 3, obb2);
	return RunInternal();
}

const Pu::vector<Pu::Vector3> & Pu::SAT::GetContacts(const OBB & obb1, const OBB & obb2)
{
	/* Prepare the buffers. */
	contacts.clear();
	FillBuffer(l1, c1);
	FillBuffer(l2, c2);
	FillBuffer(p1, obb1);
	FillBuffer(p2, obb2);

	/* Get all the points on the edges of the second OBB that intersect with the first OBB. */
	Vector3 p;
	for (Plane plane : p1)
	{
		for (Line line : l2)
		{
			if (PlaneClipLine(plane, line, p))
			{
				if (contains(obb1, p)) contacts.emplace_back(p);
			}
		}
	}

	/* Get all the points on the edges of the first OBB that intersect with the second OBB. */
	for (Plane plane : p2)
	{
		for (Line line : l1)
		{
			if (PlaneClipLine(plane, line, p))
			{
				if (contains(obb2, p)) contacts.emplace_back(p);
			}
		}
	}

	/* Calculate the relative point of impact. */
	const Vector2 i = interval(c1, n);
	const float d = (i.Y - i.X) * 0.5f - minDepth * 0.5f;
	p = obb1.Center + n * d;

	/* Transform the contact points to world space and remove duplicates. */
	TransformAndCull(p);
	return contacts;
}

bool Pu::SAT::PlaneClipLine(Plane plane, Line line, Vector3 & result)
{
	const Vector3 ab = line.End - line.Start;
	const float nda = dot(plane.N, line.Start);
	const float ndab = dot(plane.N, ab);

	if (nrlyneql(ndab, 0.0f))
	{
		const float t = (plane.D - nda) / ndab;
		if (t >= 0.0f && t <= 1.0f)
		{
			result = line.Start + ab * t;
			return true;
		}
	}

	return false;
}

void Pu::SAT::FillBuffer(Plane * buffer, const OBB & obb)
{
	const Vector3 r = obb.GetRight();
	const Vector3 u = obb.GetUp();
	const Vector3 f = obb.GetForward();

	buffer[0] = Plane{ r, dot(r, obb.Center + r * obb.Extent.X) };
	buffer[1] = Plane{ -r, dot(r, obb.Center - r * obb.Extent.X) };
	buffer[2] = Plane{ u, dot(u, obb.Center + u * obb.Extent.Y) };
	buffer[3] = Plane{ -u, dot(u, obb.Center - u * obb.Extent.Y) };
	buffer[4] = Plane{ f, dot(f, obb.Center + f * obb.Extent.Z) };
	buffer[5] = Plane{ -f, dot(f, obb.Center - f * obb.Extent.Z) };
}

void Pu::SAT::FillBuffer(Line * buffer, const Vector3 * corners)
{
	buffer[0] = Line{ corners[0], corners[1] };
	buffer[1] = Line{ corners[1], corners[5] };
	buffer[2] = Line{ corners[5], corners[4] };
	buffer[3] = Line{ corners[4], corners[0] };
	buffer[4] = Line{ corners[2], corners[3] };
	buffer[5] = Line{ corners[3], corners[7] };
	buffer[6] = Line{ corners[7], corners[6] };
	buffer[7] = Line{ corners[6], corners[2] };
	buffer[8] = Line{ corners[0], corners[2] };
	buffer[9] = Line{ corners[1], corners[3] };
	buffer[10] = Line{ corners[4], corners[6] };
	buffer[11] = Line{ corners[5], corners[7] };
}

/*
   3 +-----------+ 7
    /|          /|
   / |         / |
2 +-----------+ 6|
  |  |        |  |
  |  |        |  |
  |1 +--------|--+ 5
  | /         | /
  |/          |/
0 +-----------+ 4
*/
void Pu::SAT::FillBuffer(Vector3 * buffer, Vector3 * axes, const AABB & aabb)
{
	buffer[0] = aabb.LowerBound;
	buffer[1] = Vector3(aabb.LowerBound.X, aabb.LowerBound.Y, aabb.UpperBound.Z);
	buffer[2] = Vector3(aabb.LowerBound.X, aabb.UpperBound.Y, aabb.LowerBound.Z);
	buffer[3] = Vector3(aabb.LowerBound.X, aabb.UpperBound.Y, aabb.UpperBound.Z);
	buffer[4] = Vector3(aabb.UpperBound.X, aabb.LowerBound.Y, aabb.LowerBound.Z);
	buffer[5] = Vector3(aabb.UpperBound.X, aabb.LowerBound.Y, aabb.UpperBound.Z);
	buffer[6] = Vector3(aabb.UpperBound.X, aabb.UpperBound.Y, aabb.LowerBound.Z);
	buffer[7] = aabb.UpperBound;

	axes[0] = Vector3::Right();
	axes[1] = Vector3::Up();
	axes[2] = Vector3::Forward();
}

void Pu::SAT::FillBuffer(Vector3 * buffer, Vector3 * axes, const OBB & obb)
{
	buffer[0] = obb.Center - obb.Orientation * obb.Extent;
	buffer[1] = obb.Center - obb.Orientation * Vector3(-obb.Extent.X, obb.Extent.Y, obb.Extent.Z);
	buffer[2] = obb.Center + obb.Orientation * Vector3(obb.Extent.X, obb.Extent.Y, -obb.Extent.Z);
	buffer[3] = obb.Center + obb.Orientation * Vector3(obb.Extent.X, obb.Extent.Y, -obb.Extent.Z);
	buffer[4] = obb.Center - obb.Orientation * Vector3(obb.Extent.X, -obb.Extent.Y, obb.Extent.Z);
	buffer[5] = obb.Center - obb.Orientation * Vector3(obb.Extent.X, obb.Extent.Y, -obb.Extent.Z);
	buffer[6] = obb.Center + obb.Orientation * Vector3(obb.Extent.X, -obb.Extent.Y, obb.Extent.Z);
	buffer[7] = obb.Center + obb.Orientation * obb.Extent;

	axes[0] = obb.GetRight();
	axes[1] = obb.GetUp();
	axes[2] = obb.GetForward();
}

void Pu::SAT::TransformAndCull(Vector3 p)
{
	for (int64 i = contacts.size() - 1; i >= 0; i--)
	{
		/* Move the contact point to the world position. */
		contacts[i] += n * dot(n, p - contacts[i]);
		for (int64 j = contacts.size() - 1; j > i; j--)
		{
			/* Remove any processed contacts that are practically at the same location. */
			if (sqrdist(contacts[i], contacts[j]) < 0.0001f)
			{
				contacts.removeAt(j);
				break;
			}
		}
	}
}

bool Pu::SAT::RunInternal(void)
{
	/* Reset the state. */
	++calls;
	minDepth = maxv<float>();

	/* Initialize the last few axis. */
	for (size_t i = 0; i < 3; i++)
	{
		axes[6 + i * 3] = cross(axes[i], axes[3]);
		axes[6 + i * 3 + 1] = cross(axes[i], axes[4]);
		axes[6 + i * 3 + 2] = cross(axes[i], axes[5]);
	}

	/* Check for overlap on all the axis. */
	for (Vector3 axis : axes)
	{
		/*
		Some of the axes from the input boxes might be equal.
		This causes the cross product to return zero, so ignore those axes.
		*/
		if (nrlyneql(axis.LengthSquared(), 1.0f, 0.001f)) continue;

		const Vector2 a = interval(c1, axis);
		const Vector2 b = interval(c2, axis);

		/* The current axis doesn't overlap, we found a seperating axis. */
		if (b.X > a.Y || a.X > b.Y) return false;

		/* Check if the intersection depth is less than the previous one. */
		const float d = (a.Y - a.X) + (b.Y - b.X) - (max(a.Y, b.Y) - min(a.X, b.X));
		if (d < minDepth)
		{
			n = b.X < a.X ? -axis : axis;
			minDepth = d;
		}
	}

	return true;
}