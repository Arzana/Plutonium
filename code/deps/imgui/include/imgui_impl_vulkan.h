// dear imgui: Renderer for Vulkan
// This needs to be used along with a Platform Binding (e.g. GLFW, SDL, Win32, custom..)

// Missing features:
//  [ ] Renderer: User texture binding. Changes of ImTextureID aren't supported by this binding! See https://github.com/ocornut/imgui/pull/914

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// The aim of imgui_impl_vulkan.h/.cpp is to be usable in your engine without any modification.
// IF YOU FEEL YOU NEED TO MAKE ANY CHANGE TO THIS CODE, please share them and your feedback at https://github.com/ocornut/imgui/

// Important note to the reader who wish to integrate imgui_impl_vulkan.cpp/.h in their own engine/app.
// - Common ImGui_ImplVulkan_XXX functions and structures are used to interface with imgui_impl_vulkan.cpp/.h.
//   You will use those if you want to use this rendering back-end in your engine/app.
// - Helper ImGui_ImplVulkanH_XXX functions and structures are only used by this example (main.cpp) and by 
//   the back-end itself (imgui_impl_vulkan.cpp), but should PROBABLY NOT be used by your own engine/app code.
// Read comments in imgui_impl_vulkan.h.

#pragma once

#include "Graphics/Vulkan/VulkanObjects.h"

// Initialization data, for ImGui_ImplVulkan_Init()
// [Please zero-clear before use!]
struct ImGui_ImplVulkan_InitInfo
{
    Pu::InstanceHndl			Instance;
    Pu::PhysicalDeviceHndl		PhysicalDevice;
    Pu::DeviceHndl				Device;
    uint32_t					QueueFamily;
    Pu::QueueHndl				Queue;
    Pu::PipelineCacheHndl		PipelineCache;
    Pu::DescriptorPoolHndl		DescriptorPool;
    uint32_t					MinImageCount;          // >= 2
    uint32_t					ImageCount;             // >= MinImageCount
    const Pu::AllocationCallbacks* Allocator;
    void(*CheckVkResultFn)(Pu::VkApiResult err);
};

// Called by user code
IMGUI_IMPL_API bool     ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info, Pu::RenderPassHndl render_pass);
IMGUI_IMPL_API void     ImGui_ImplVulkan_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplVulkan_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, Pu::CommandBufferHndl command_buffer);
IMGUI_IMPL_API bool     ImGui_ImplVulkan_CreateFontsTexture(Pu::CommandBufferHndl command_buffer);
IMGUI_IMPL_API void     ImGui_ImplVulkan_DestroyFontUploadObjects();
IMGUI_IMPL_API void     ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count); // To override MinImageCount after initialization (e.g. if swap chain is recreated)


//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT used by your own engine/app.)
//-------------------------------------------------------------------------
// You probably do NOT need to use or care about those functions.
// Those functions only exist because:
//   1) they facilitate the readability and maintenance of the multiple main.cpp examples files.
//   2) the upcoming multi-viewport feature will need them internally.
// Generally we avoid exposing any kind of superfluous high-level helpers in the bindings,
// but it is too much code to duplicate everywhere so we exceptionally expose them.
//
// Your engine/app will likely _already_ have code to setup all that stuff (swap chain, render pass, frame buffers, etc.).
// You may read this code to learn about Vulkan, but it is recommended you use you own custom tailored code to do equivalent work.
// (The ImGui_ImplVulkanH_XXX functions do not interact with any of the state used by the regular ImGui_ImplVulkan_XXX functions)
//-------------------------------------------------------------------------

struct ImGui_ImplVulkanH_Frame;
struct ImGui_ImplVulkanH_Window;

// Helpers
IMGUI_IMPL_API void                 ImGui_ImplVulkanH_CreateWindow(Pu::InstanceHndl instance, Pu::PhysicalDeviceHndl physical_device, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wnd, uint32_t queue_family, const Pu::AllocationCallbacks* allocator, int w, int h, uint32_t min_image_count);
IMGUI_IMPL_API void                 ImGui_ImplVulkanH_DestroyWindow(Pu::InstanceHndl instance, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wnd, const Pu::AllocationCallbacks* allocator);
IMGUI_IMPL_API Pu::SurfaceFormat    ImGui_ImplVulkanH_SelectSurfaceFormat(Pu::PhysicalDeviceHndl physical_device, Pu::SurfaceHndl surface, const Pu::Format* request_formats, int request_formats_count, Pu::ColorSpace request_color_space);
IMGUI_IMPL_API Pu::PresentMode      ImGui_ImplVulkanH_SelectPresentMode(Pu::PhysicalDeviceHndl physical_device, Pu::SurfaceHndl surface, const Pu::PresentMode* request_modes, int request_modes_count);
IMGUI_IMPL_API int                  ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(Pu::PresentMode present_mode);

// Helper structure to hold the data needed by one rendering frame
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own engine/app.)
// [Please zero-clear before use!]
struct ImGui_ImplVulkanH_Frame
{
    Pu::CommandPoolHndl       CommandPool;
    Pu::CommandBufferHndl     CommandBuffer;
    Pu::FenceHndl             Fence;
    Pu::ImageHndl             Backbuffer;
    Pu::ImageViewHndl         BackbufferView;
    Pu::FramebufferHndl       Framebuffer;
};

struct ImGui_ImplVulkanH_FrameSemaphores
{
    Pu::SemaphoreHndl         ImageAcquiredSemaphore;
    Pu::SemaphoreHndl         RenderCompleteSemaphore;
};

// Helper structure to hold the data needed by one rendering context into one OS window
// (Used by example's main.cpp. Used by multi-viewport features. Probably NOT used by your own engine/app.)
struct ImGui_ImplVulkanH_Window
{
    int                 Width;
    int                 Height;
    Pu::SwapchainHndl   Swapchain;
    Pu::SurfaceHndl     Surface;
    Pu::SurfaceFormat	SurfaceFormat;
    Pu::PresentMode     PresentMode;
    Pu::RenderPassHndl  RenderPass;
    bool                ClearEnable;
    Pu::ClearValue      ClearValue;
    uint32_t            FrameIndex;             // Current frame being rendered to (0 <= FrameIndex < FrameInFlightCount)
    uint32_t            ImageCount;             // Number of simultaneous in-flight frames (returned by vkGetSwapchainImagesKHR, usually derived from min_image_count)
    uint32_t            SemaphoreIndex;         // Current set of swapchain wait semaphores we're using (needs to be distinct from per frame data)
    ImGui_ImplVulkanH_Frame*            Frames;
    ImGui_ImplVulkanH_FrameSemaphores*  FrameSemaphores;

    ImGui_ImplVulkanH_Window()
		: ClearValue{ 0.0f, 0.0f, 0.0f, 0.0f }
    { 
        memset(this, 0, sizeof(*this)); 
        PresentMode = (Pu::PresentMode)0x7FFFFFFF;
        ClearEnable = true;
    }
};

