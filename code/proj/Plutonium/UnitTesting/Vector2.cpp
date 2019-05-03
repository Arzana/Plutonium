#include "stdafx.h"
#include "CppUnitTest.h"
#include <ctime>
#include <Core/Math/Vector2.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTesting
{
	TEST_CLASS(Vector2)
	{
	public:
		TEST_METHOD(DefaultCtor)
		{
			const Pu::Vector2 v;
			Assert::AreEqual(0.0f, v.X, L"Vector2.X was not default initialized to zero!");
			Assert::AreEqual(0.0f, v.Y, L"Vector2.Y was not default initialized to zero!");
		}

		TEST_METHOD(SingleValueCtor)
		{
			const float value = RandomFloat();
			const Pu::Vector2 v{ value };

			Assert::AreEqual(value, v.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(value, v.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(DoubleValueCtor)
		{
			const float valueX = RandomFloat();
			const float valueY = RandomFloat();
			const Pu::Vector2 v{ valueX, valueY };

			Assert::AreEqual(valueX, v.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(valueY, v.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(CopyCtor)
		{
			const Pu::Vector2 origional{ -2.3f, 0.6f };
			const Pu::Vector2 copy{ origional };

			Assert::AreEqual(origional.X, copy.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(origional.Y, copy.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(MoveCtor)
		{
			Pu::Vector2 origional{ 0.3f, 0.6f };
			const Pu::Vector2 copy{ std::move(origional) };

			Assert::AreEqual(origional.X, copy.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(origional.Y, copy.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(Negate)
		{
			const Pu::Vector2 v = RandomVector2();
			const Pu::Vector2 w = -v;

			Assert::AreEqual(-v.X, w.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(-v.Y, w.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(Subtract)
		{
			const Pu::Vector2 v = RandomVector2();
			const Pu::Vector2 w = RandomVector2();
			const Pu::Vector2 u = v - w;

			Assert::AreEqual(v.X - w.X, u.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(v.Y - w.Y, u.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(Add)
		{
			const Pu::Vector2 v = RandomVector2();
			const Pu::Vector2 w = RandomVector2();
			const Pu::Vector2 u = v + w;

			Assert::AreEqual(v.X + w.X, u.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(v.Y + w.Y, u.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(MultiplyByScalar)
		{
			const Pu::Vector2 v = RandomVector2();
			const float scalar = RandomFloat();
			const Pu::Vector2 w = v * scalar;

			Assert::AreEqual(v.X * scalar, w.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(v.Y * scalar, w.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(MultiplyByVector)
		{
			const Pu::Vector2 v = RandomVector2();
			const Pu::Vector2 w = RandomVector2();
			const Pu::Vector2 u = v * w;

			Assert::AreEqual(v.X * w.X, u.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(v.Y * w.Y, u.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(DivideByScalar)
		{
			const Pu::Vector2 v = RandomVector2();
			const float scalar = RandomFloat();
			const Pu::Vector2 w = v / scalar;

			Assert::AreEqual(v.X / scalar, w.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(v.Y / scalar, w.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(DivideByVector)
		{
			const Pu::Vector2 v = RandomVector2();
			const Pu::Vector2 w = RandomVector2();
			const Pu::Vector2 u = v / w;

			Assert::AreEqual(v.X / w.X, u.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(v.Y / w.Y, u.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(Equality)
		{
			const Pu::Vector2 v = RandomVector2();
			Assert::IsTrue(v == v, L"Vector2 failed equality test!");
		}

		TEST_METHOD(Difference)
		{
			const Pu::Vector2 v = RandomVector2();
			const Pu::Vector2 w = -v;
			Assert::IsFalse(v != v, L"Vector2 failed equality test!");
		}

		TEST_METHOD(FromAngle)
		{
			const Pu::Vector2 v = Pu::Vector2::FromAngle(0.0f);
			Assert::AreEqual(1.0f, v.X, L"Vector2.X was not initialized to the correct value!");
			Assert::AreEqual(0.0f, v.Y, L"Vector2.Y was not initialized to the correct value!");
		}

		TEST_METHOD(Angle)
		{
			const Pu::Vector2 v{ -1.0f, 0.0f };
			Assert::AreEqual(Pu::PI, v.Angle(), L"Vector2 didn't create the desired angle!");
		}

	private:
		float RandomFloat(void)
		{
			static bool shouldSeed = true;
			if (shouldSeed)
			{
				shouldSeed = false;
				srand(static_cast<unsigned int>(time(0)));
			}

			return -1000.0f + (rand() / (RAND_MAX / (2000.0f)));
		}

		Pu::Vector2 RandomVector2(void)
		{
			return Pu::Vector2{ RandomFloat(), RandomFloat() };
		}
	};
}