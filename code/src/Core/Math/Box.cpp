#include "Core\Math\Box.h"
#include "Core\Math\VInterpolation.h"

using namespace Plutonium;

Box Plutonium::Box::Mix(const Box & first, const Box & second, float a)
{
	return Box(lerp(first.Position, second.Position, a), lerp(first.Size, second.Size, a));
}

Box Plutonium::Box::Merge(const Box & first, const Box & second)
{
	if (first.IsEmpty()) return second;
	if (second.IsEmpty()) return first;

	const float r = max(first.GetRight(), second.GetRight());
	const float l = min(first.GetLeft(), second.GetLeft());
	const float t = max(first.GetTop(), second.GetTop());
	const float d = min(first.GetBottom(), second.GetBottom());
	const float b = max(first.GetBack(), second.GetBack());
	const float f = min(first.GetFront(), second.GetFront());
	return Box(l, d, f, r - l, t - d, b - f);
}

void Plutonium::Box::Inflate(float horizontal, float vertical, float depth)
{
	horizontal *= 0.5f;
	vertical *= 0.5f;
	depth *= 0.5f;

	if (Size.X >= 0.0f)
	{
		Position.X -= horizontal;
		Size.X += horizontal;
	}
	else
	{
		Position.X += horizontal;
		Size.X -= horizontal;
	}

	if (Size.Y >= 0.0f)
	{
		Position.Y -= vertical;
		Size.Y += vertical;
	}
	else
	{
		Position.Y += vertical;
		Size.Y -= vertical;
	}

	if (Size.Z >= 0.0f)
	{
		Position.Z -= depth;
		Size.Z += depth;
	}
	else
	{
		Position.Z += depth;
		Size.Z -= depth;
	}
}

void Plutonium::Box::Square(void)
{
	Size = Vector3(max(max(Size.X, Size.Y), Size.Z));
}

bool Plutonium::Box::Contains(Vector3 point) const
{
	return GetLeft() <= point.X && GetRight() >= point.X
		&& GetTop() <= point.Y && GetBottom() >= point.Y
		&& GetFront() <= point.Z && GetBack() >= point.Z;
}

bool Plutonium::Box::Contains(const Box & b) const
{
	return GetLeft() <= b.GetLeft() && GetRight() >= b.GetRight()
		&& GetTop() <= b.GetTop() && GetBottom() >= b.GetBottom()
		&& GetFront() <= b.GetFront() && GetBack() >= b.GetBack();
}

bool Plutonium::Box::Overlaps(const Box & b) const
{
	return GetLeft() <= b.GetRight() && GetRight() >= b.GetLeft()
		&& GetTop() <= b.GetBottom() && GetBottom() >= b.GetTop()
		&& GetFront() <= b.GetBack() && GetBack() >= b.GetFront();
}

Box Plutonium::Box::GetOverlap(const Box & b) const
{
	const float xl = max(GetLeft(), b.GetLeft());
	const float xs = min(GetRight(), b.GetRight());
	if (xs < xl) return Box();

	const float yl = max(GetTop(), b.GetTop());
	const float ys = min(GetBottom(), b.GetBottom());
	if (ys < yl) return Box();

	const float zl = max(GetFront(), b.GetFront());
	const float zs = min(GetBack(), b.GetBack());
	if (zs < zl) return Box();

	return Box(xl, yl, zl, xs - xl, ys - yl, zs - zl);
}

bool Plutonium::Box::HitTestRay(Vector3 origin, Vector3 dir, const Matrix & world, float * dist)
{
	/* http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/ */
	float near = minv<float>();
	float far = maxv<float>();

	Vector3 max = Position + Size;
	Vector3 delta = Position - origin;
	Vector3 axis[3] = { world.GetRight(), world.GetUp(), world.GetBackward() };

	for (size_t i = 0; i < 3; i++)
	{
		Vector3 curAxis = normalize(axis[i]);

		float e = dot(curAxis, delta);
		float f = dot(curAxis, dir);

		if (fabs(f) > EPSILON)
		{
			float t1 = (e + Position.f[i]) / f;
			float t2 = (e + max.f[i]) / f;

			if (t1 > t2)
			{
				float tmp = t1;
				t1 = t2;
				t2 = tmp;
			}

			if (t2 < far) far = t2;
			if (t1 > near) near = t1;

			if (far < near) return false;
		}
		else if (-e + Position.f[i] > 0.0f || -e + max.f[i] < 0.0f) return false;
	}

	if (dist) *dist = near;
	return true;
}