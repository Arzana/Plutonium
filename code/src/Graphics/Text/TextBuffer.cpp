#include "Graphics/Text/TextBuffer.h"
#include "Graphics/VertexLayouts/Image2D.h"

Pu::TextBuffer::TextBuffer(LogicalDevice & device, size_t initialSize)
	: device(device), view(nullptr)
{
	AllocBuffer(initialSize);
}

Pu::TextBuffer::TextBuffer(TextBuffer && value)
	: device(value.device), buffer(value.buffer), view(value.view)
{
	value.buffer = nullptr;
	value.view = nullptr;
}

Pu::TextBuffer & Pu::TextBuffer::operator=(TextBuffer && other)
{
	if (this != &other)
	{
		Destroy();

		device = std::move(other.device);
		buffer = other.buffer;
		view = other.view;

		other.buffer = nullptr;
		other.view = nullptr;
	}

	return *this;
}

void Pu::TextBuffer::Update(CommandBuffer & cmdBuffer)
{
	buffer->Update(cmdBuffer);
}

void Pu::TextBuffer::SetText(const ustring & str, const Font &font)
{
	/* 6 vertices per quad of Image2D type per glyph. */
	const size_t size = str.length() * 6;
	const float lh = static_cast<float>(font.GetLineSpace());

	/* If we can't store the specific string in this buffer, reallocate the buffer. */
	if (size > buffer->GetSize()) ReallocBuffer(size);

	/* Start the memory copy into the CPU/GPU buffer. */
	buffer->BeginMemoryTransfer();
	Image2D *data = reinterpret_cast<Image2D*>(buffer->GetHostMemory());

	Vector2 adder(0.0f, lh);
	size_t i = 0;
	for (char32 key : str)
	{
		/* For newlines just add the line space to the adder and reset the horizontal adder. */
		if (key == U'\n')
		{
			adder.X = 0.0f;
			adder.Y += lh;
			continue;
		}
		else if (key == U'\r') continue;

		/* Gets the positional and texture information from the glyph. */
		const Glyph &glyph = font.GetGlyph(key);
		const Vector2 tl = adder + glyph.Bearing;
		const Vector2 br = tl + glyph.Size;
		const Vector2 uv1 = glyph.Bounds.Position;
		const Vector2 uv2 = glyph.Bounds.Position + glyph.Bounds.Size;

		/* Copy the vertex data to the buffer. */
		data[i++] = Image2D(tl, uv1);
		data[i++] = Image2D(tl.X, br.Y, uv1.X, uv2.Y);
		data[i++] = Image2D(br, uv2);
		data[i++] = Image2D(tl, uv1);
		data[i++] = Image2D(br, uv2);
		data[i++] = Image2D(br.X, tl.Y, uv2.X, uv1.Y);

		adder.X += static_cast<float>(glyph.Advance);
	}

	buffer->EndMemoryTransfer();

	/* Delete the old buffer view if needed and create a new buffer view for rendering. */
	if (view) delete view;
	view = new BufferView(*buffer, 0, i, sizeof(Image2D));
}

void Pu::TextBuffer::ReallocBuffer(size_t newSize)
{
	Log::Warning("Reallocating text buffer from %zu to %zu bytes!", buffer->GetSize(), newSize);

	if (buffer) delete buffer;
	AllocBuffer(newSize);
}

void Pu::TextBuffer::AllocBuffer(size_t size)
{
	/* Currently we're not index rendering the text as it doesn't improve our memory usage by much but it increases complexity. */
	buffer = new DynamicBuffer(device, size, BufferUsageFlag::TransferDst | BufferUsageFlag::VertexBuffer);
}

void Pu::TextBuffer::Destroy(void)
{
	if (buffer) delete buffer;
	if (view) delete view;
}