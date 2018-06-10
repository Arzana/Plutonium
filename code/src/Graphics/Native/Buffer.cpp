#include "Graphics\Native\Buffer.h"
#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Core\EnumUtils.h"

using namespace Plutonium;

Plutonium::Buffer::Buffer(WindowHandler wnd, BindTarget target)
	: wnd(wnd), hndlr(0), size(0), type(_CrtEnum2Int(target))
{
	/* Generate a new handler for the buffer. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glGenBuffers(1, &hndlr);
	}));
}

Plutonium::Buffer::~Buffer(void) noexcept
{
	/* If the buffer has not yet been released, release it. */
	if (hndlr)
	{
		hndlr = 0;
		glDeleteBuffers(1, &hndlr);
		_CrtUpdateUsedGPUMemory(-bsize);
	}
}

void Plutonium::Buffer::Bind(void) const
{
	/* Error check if glDeleteBuffers has already been called and if the target is correct. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Invalid bind target specified!");

	/* Bind the buffer thread safe. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindBuffer(type, hndlr);
	}));
}

void Plutonium::Buffer::BufferData(BufferUsage usage, size_t size, const void * data)
{
	/* Error check for invalid handler and not bound buffer. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Cannot set data for unbound buffer %zu!", hndlr);

	/* Buffer the data thread safe. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindBuffer(type, hndlr);
		glBufferData(type, size, data, static_cast<GLenum>(usage));
	}));

	LOG("%s Buffer 0x%04x allocated (%zu bytes).", _CrtGetBufferUsageStr(usage), hndlr, size);
	_CrtUpdateUsedGPUMemory(bsize = static_cast<int64>(size));
}

void Plutonium::Buffer::BufferSubData(size_t size, const void * data, bool sizeUpdated)
{
	/* Error check for invalid handler and not bound buffer. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Cannot set data for unbound buffer %zu!", hndlr);

	/* Buffer the sub data thread safe. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindBuffer(type, hndlr);
		glBufferSubData(type, 0, size, data);
	}));

	/* Update GPU diag if needed. */
	if (sizeUpdated)
	{
		_CrtUpdateUsedGPUMemory(-bsize);
		_CrtUpdateUsedGPUMemory(bsize = static_cast<int64>(size));
	}
}