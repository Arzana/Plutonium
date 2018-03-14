#include "Graphics\Native\Buffer.h"

Plutonium::Buffer::Buffer(void)
	: hndlr(0), size(0), type(0)
{
	/* Generate a new handler for the buffer. */
	glGenBuffers(1, &hndlr);
}

Plutonium::Buffer::~Buffer(void) noexcept
{
	/* If the buffer has not yet been released, release it. */
	if (hndlr)
	{
		hndlr = 0;
		glDeleteBuffers(1, &hndlr);
	}
}

void Plutonium::Buffer::Bind(void)
{
	/* Error check if glDeleteBuffers has already been called and if the target is correct. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Invalid bind target specified!");

	glBindBuffer(type, hndlr);
}

void Plutonium::Buffer::Bind(BindTarget target)
{
	/* Set or reset the type and bind buffer. */
	type = static_cast<GLenum>(target);
	Bind();
}

void Plutonium::Buffer::BufferData(BufferUsage usage, size_t size, const void * data)
{
	/* Error check for invalid handler and not bound buffer. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Cannot set data for unbound buffer %zu!", hndlr);

	LOG("%s Buffer allocated (%zu bytes).", _CrtGetBufferUsageStr(usage), size);
	glBufferData(type, size, data, static_cast<GLenum>(usage));
}

void Plutonium::Buffer::BufferSubData(size_t size, const void * data)
{
	/* Error check for invalid handler and not bound buffer. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Cannot set data for unbound buffer %zu!", hndlr);

	glBufferSubData(type, 0, size, data);
}