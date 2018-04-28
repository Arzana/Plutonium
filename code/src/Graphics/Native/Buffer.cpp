#include "Graphics\Native\Buffer.h"
#include "Graphics\Diagnostics\DeviceInfo.h"
#include "Core\Threading\PuThread.h"
#include <atomic>

using namespace Plutonium;

struct BufferInvoker
{
public:
	std::atomic_bool invoked;

	BufferInvoker(Buffer *buffer)
		: buffer(buffer), invoked(false)
	{}

	BufferInvoker(Buffer *buffer, size_t size, const void *data)
		: buffer(buffer), invoked(false), size(size), data(data)
	{}

	BufferInvoker(Buffer *buffer, size_t size, const void *data, BufferUsage usage)
		: buffer(buffer), invoked(false), size(size), data(data), usage(usage)
	{}

	void InvokeGenerate(WindowHandler, EventArgs)
	{
		glGenBuffers(1, &buffer->hndlr);
		invoked.store(true);
	}

	void InvokeBind(WindowHandler, EventArgs)
	{
		glBindBuffer(buffer->type, buffer->hndlr);
		invoked.store(true);
	}

	void InvokeBufferData(WindowHandler, EventArgs)
	{
		glBindBuffer(buffer->type, buffer->hndlr);
		glBufferData(buffer->type, size, data, static_cast<GLenum>(usage));
		invoked.store(true);
	}

	void InvokeBufferSubData(WindowHandler, EventArgs)
	{
		glBindBuffer(buffer->type, buffer->hndlr);
		glBufferSubData(buffer->type, 0, size, data);
		invoked.store(true);
	}

private:
	Buffer *buffer;
	size_t size;
	const void *data;
	BufferUsage usage;
};

Plutonium::Buffer::Buffer(WindowHandler wnd)
	: wnd(wnd), hndlr(0), size(0), type(0)
{
	/* Generate a new handler for the buffer. */
	BufferInvoker obj(this);
	wnd->Invoke(new Invoker(&obj, &BufferInvoker::InvokeGenerate));
	while (!obj.invoked.load()) PuThread::Sleep(10);
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

void Plutonium::Buffer::Bind(void)
{
	/* Error check if glDeleteBuffers has already been called and if the target is correct. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Invalid bind target specified!");

	/* Bind the buffer thread safe. */
	BufferInvoker obj(this);
	wnd->Invoke(new Invoker(&obj, &BufferInvoker::InvokeBind));
	while (!obj.invoked.load()) PuThread::Sleep(10);
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

	/* Buffer the data thread safe. */
	BufferInvoker obj(this, size, data, usage);
	wnd->Invoke(new Invoker(&obj, &BufferInvoker::InvokeBufferData));
	while (!obj.invoked.load()) PuThread::Sleep(10);

	LOG("%s Buffer 0x%04x allocated (%zu bytes).", _CrtGetBufferUsageStr(usage), hndlr, size);
	_CrtUpdateUsedGPUMemory(bsize = static_cast<int64>(size));
}

void Plutonium::Buffer::BufferSubData(size_t size, const void * data, bool sizeUpdated)
{
	/* Error check for invalid handler and not bound buffer. */
	ASSERT_IF(!hndlr, "Cannot bind released buffer!");
	ASSERT_IF(!type, "Cannot set data for unbound buffer %zu!", hndlr);

	/* Buffer the sub data thread safe. */
	BufferInvoker obj(this, size, data);
	wnd->Invoke(new Invoker(&obj, &BufferInvoker::InvokeBufferSubData));
	while (!obj.invoked.load()) PuThread::Sleep(10);

	/* Update GPU diag if needed. */
	if (sizeUpdated)
	{
		_CrtUpdateUsedGPUMemory(-bsize);
		_CrtUpdateUsedGPUMemory(bsize = static_cast<int64>(size));
	}
}