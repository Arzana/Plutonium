#include "Graphics\Native\Buffer.h"
#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Core\EnumUtils.h"

using namespace Plutonium;

Plutonium::Buffer::Buffer(WindowHandler wnd, BindTarget target)
	: wnd(wnd), hndlr(0), size(0), bsize(0), type(target)
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

	/* Bind the buffer thread safe. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindBuffer(_CrtEnum2Int(type), hndlr);
	}));
}

void Plutonium::Buffer::BufferData(BufferUsage usage, size_t sizeBytes, const void * data)
{
	/* Error check for invalid handler and not bound buffer. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");

	/* Buffer the data thread safe. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindBuffer(_CrtEnum2Int(type), hndlr);
		glBufferData(_CrtEnum2Int(type), sizeBytes, data, _CrtEnum2Int(usage));
	}));

	LOG("%s Buffer 0x%04x allocated (%zu bytes).", _CrtGetBufferUsageStr(usage), hndlr, sizeBytes);
	_CrtUpdateUsedGPUMemory(bsize = static_cast<int64>(sizeBytes));
}

void Plutonium::Buffer::BufferSubData(size_t sizeBytes, const void * data, bool sizeUpdated)
{
	/* Error check for invalid handler and not bound buffer. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");

	/* Buffer the sub data thread safe. */
	wnd->InvokeWait(Invoker([&](WindowHandler, EventArgs)
	{
		glBindBuffer(_CrtEnum2Int(type), hndlr);
		glBufferSubData(_CrtEnum2Int(type), 0, sizeBytes, data);
	}));

	/* Update GPU diag if needed. */
	if (sizeUpdated)
	{
		_CrtUpdateUsedGPUMemory(-bsize);
		_CrtUpdateUsedGPUMemory(bsize = static_cast<int64>(sizeBytes));
	}
}