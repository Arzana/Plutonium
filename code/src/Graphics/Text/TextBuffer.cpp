#include "Graphics/Text/TextBuffer.h"
#include "Graphics/VertexLayouts/Image2D.h"

Pu::TextBuffer::TextBuffer(LogicalDevice & device, size_t initialSize)
	: device(&device), count(0)
{
	AllocBuffer(initialSize * sizeof(Image2D) * 6);
}

Pu::TextBuffer::TextBuffer(TextBuffer && value)
	: device(value.device), buffer(value.buffer), count(value.count)
{
	value.buffer = nullptr;
}

Pu::TextBuffer & Pu::TextBuffer::operator=(TextBuffer && other)
{
	if (this != &other)
	{
		Destroy();

		device = other.device;
		buffer = other.buffer;
		count = other.count;

		other.buffer = nullptr;
	}

	return *this;
}

void Pu::TextBuffer::Update(CommandBuffer & cmdBuffer)
{
	buffer->Update(cmdBuffer);
}

void Pu::TextBuffer::SetText(const ustring & str, const Font & font, const GameWindow & wnd)
{
	/* 6 vertices per quad of Image2D type per glyph. */
	const size_t size = str.length() * sizeof(Image2D) * 6;
	const float lh = static_cast<float>(font.GetLineSpace());

	/* If we can't store the specific string in this buffer, reallocate the buffer. */
	if (size > buffer->GetSize()) ReallocBuffer(size);

	/* Start the memory copy into the CPU/GPU buffer. */
	buffer->BeginMemoryTransfer();
	Image2D *data = reinterpret_cast<Image2D*>(buffer->GetHostMemory());

	Vector2 adder(0.0f, lh);

	bool firstChar = true;
	char32 old = U'\0';

	count = 0;
	for (char32 key : str)
	{
		/* Add kerning advance. */
		if (!firstChar) adder.X += font.GetKerning(old, key);
		else firstChar = true;

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
		const Vector2 tl = wnd.ToLinearClipSpace(Vector4(adder + glyph.Bearing, 0.0f, 1.0f)).XY;
		const Vector2 br = wnd.ToLinearClipSpace(Vector4(adder + glyph.Bearing + glyph.Size, 0.0f, 1.0f)).XY;

		/* Copy the vertex data to the buffer. */
		data[count++] = Image2D(tl, glyph.U);
		data[count++] = Image2D(tl.X, br.Y, glyph.U.X, glyph.V.Y);
		data[count++] = Image2D(br, glyph.V);
		data[count++] = Image2D(tl, glyph.U);
		data[count++] = Image2D(br, glyph.V);
		data[count++] = Image2D(br.X, tl.Y, glyph.V.X, glyph.U.Y);

		adder.X += static_cast<float>(glyph.Advance);
	}

	buffer->EndMemoryTransfer();
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
	buffer = new DynamicBuffer(*device, size, BufferUsageFlags::TransferDst | BufferUsageFlags::VertexBuffer);
}

void Pu::TextBuffer::Destroy(void)
{
	if (buffer) delete buffer;
}