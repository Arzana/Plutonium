#include "Graphics/Vulkan/Framebuffer.h"
#include "Graphics/Vulkan/ImageView.h"

Pu::Framebuffer::Framebuffer(Framebuffer && value)
	: parent(value.parent), hndl(value.hndl)
{
	value.hndl = nullptr;
}

Pu::Framebuffer & Pu::Framebuffer::operator=(Framebuffer && other)
{
	if (this != &other)
	{
		Destroy();

		parent = std::move(other.parent);
		hndl = other.hndl;

		other.hndl = nullptr;
	}

	return *this;
}

Pu::Framebuffer::Framebuffer(LogicalDevice & device, RenderPassHndl renderPass, Extent2D dimensions, vector<ImageView*> attachments)
	: parent(device)
{
	const vector<ImageViewHndl> handles(attachments.select<ImageViewHndl>([](const ImageView *cur) { return cur->hndl; }));

	FramebufferCreateInfo createInfo(renderPass, dimensions.Width, dimensions.Height);
	createInfo.AttachmentCount = static_cast<uint32>(attachments.size());
	createInfo.Attachments = handles.data();

	VK_VALIDATE(parent.vkCreateFramebuffer(parent.hndl, &createInfo, nullptr, &hndl), PFN_vkCreateFramebuffer);
}

void Pu::Framebuffer::Destroy(void)
{
	if (hndl) parent.vkDestroyFramebuffer(parent.hndl, hndl, nullptr);
}