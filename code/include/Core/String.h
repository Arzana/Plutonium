#pragma once
#include "Core\Math\Vector2.h"
#include "Core\Math\Matrix.h"

namespace Plutonium
{
	/* Defines an ease of use string type. */
	struct String
	{
	public:
		/* Initializes a new empty instance of a string. */
		String(void);
		/* Initializes a new instance of a string from a premade C-style string. */
		String(_In_ const char *value);
		/* Moves the string to a new memory location. */
		String(_In_ String &&value);
		/* Releases the resources allocated by the string. */
		~String(void);

		/* Moves the string to a new memory location. */
		_Check_return_ String& operator =(_In_ String &&other);
		/* Initializes a new instance of a string from a premade C-style string. */
		_Check_return_ String operator =(_In_ const char *value);
		/* Gets the underlying string. */
		_Check_return_ operator const char* (void) const;

		/* Adds a ease of use string to the string. */
		_Check_return_ String& operator +=(_In_ const String &other);
		/* Adds a C-style string to the string. */
		_Check_return_ String& operator +=(_In_ const char *value);
		/* Adds a pointer type to the string. */
		_Check_return_ String& operator +=(_In_ const void *value);
		/* Adds an integer to the string. */
		_Check_return_ String& operator +=(_In_ int value);
		/* Adds a floating point to the string. */
		_Check_return_ String& operator +=(_In_ float value);
		/* Adds a 2D vector to the string. */
		_Check_return_ String& operator +=(_In_ Vector2 value);
		/* Adds a 3D vector to the string. */
		_Check_return_ String& operator +=(_In_ Vector3 value);
		/* Adds a 4D vector to the string. */
		_Check_return_ String& operator +=(_In_ Vector4 value);
		/* Adds a matrix to the string. */
		_Check_return_ String& operator +=(_In_ const Matrix &value);

		/* Clears the data withing the string. */
		void Clear(void);

		/* Gets the length of the string. */
		_Check_return_ inline size_t Length(void) const
		{
			return len;
		}

	private:
		char *str;
		size_t len;
		bool heap;

		void MrgInto(const char *value);
	};
}