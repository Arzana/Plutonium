#include "Graphics/Vulkan/Framebuffer.h"
#include "Graphics/Vulkan/ImageView.h"

Pu::Framebuffer::Framebuffer(LogicalDevice & device, const Renderpass & renderPass, Extent2D dimensions, const vector<const ImageView*>& attachments)
	: parent(&device), area(dimensions.Width, dimensions.Height)
{
	const vector<ImageViewHndl> handles(attachments.select<ImageViewHndl>([](const ImageView *cur) { return cur->hndl; }));

	FramebufferCreateInfo createInfo(renderPass.hndl, dimensions.Width, dimensions.Height);
	createInfo.AttachmentCount = static_cast<uint32>(attachments.size());
	createInfo.Attachments = handles.data();

	VK_VALIDATE(parent->vkCreateFramebuffer(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateFramebuffer);
}

/* This constructor is used for Imgui. */
Pu::Framebuffer::Framebuffer(LogicalDevice & device, RenderPassHndl renderPass, Extent2D dimensions, ImageViewHndl attachment)
	: parent(&device), area(dimensions.Width, dimensions.Height)
{
	FramebufferCreateInfo createInfo(renderPass, dimensions.Width, dimensions.Height);
	createInfo.AttachmentCount = 1;
	createInfo.Attachments = &attachment;

	VK_VALIDATE(parent->vkCreateFramebuffer(parent->hndl, &createInfo, nullptr, &hndl), PFN_vkCreateFramebuffer);
}

Pu::Framebuffer::Framebuffer(Framebuffer && value)
	: parent(value.parent), hndl(value.hndl), area(value.area)
{
	value.hndl = nullptr;
}

Pu::Framebuffer & Pu::Framebuffer::operator=(Framebuffer && other)
{
	if (this != &other)
	{
		Destroy();

		parent = other.parent;
		hndl = other.hndl;
		area = other.area;

		other.hndl = nullptr;
	}

	return *this;
}

void Pu::Framebuffer::Destroy(void)
{
	if (hndl) parent->vkDestroyFramebuffer(parent->hndl, hndl, nullptr);
}