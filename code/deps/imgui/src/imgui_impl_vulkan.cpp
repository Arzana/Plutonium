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

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2019-04-30: Vulkan: Added support for special ImDrawCallback_ResetRenderState callback to reset render state.
//  2019-04-04: *BREAKING CHANGE*: Vulkan: Added ImageCount/MinImageCount fields in ImGui_ImplVulkan_InitInfo, required for initialization (was previously a hard #define IMGUI_VK_QUEUED_FRAMES 2). Added ImGui_ImplVulkan_SetMinImageCount().
//  2019-04-04: Vulkan: Added VkInstance argument to ImGui_ImplVulkanH_CreateWindow() optional helper.
//  2019-04-04: Vulkan: Avoid passing negative coordinates to vkCmdSetScissor, which debug validation layers do not like.
//  2019-04-01: Vulkan: Support for 32-bit index buffer (#define ImDrawIdx unsigned int).
//  2019-02-16: Vulkan: Viewport and clipping rectangles correctly using draw_data->FramebufferScale to allow retina display.
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-08-25: Vulkan: Fixed mishandled VkSurfaceCapabilitiesKHR::maxImageCount=0 case.
//  2018-06-22: Inverted the parameters to ImGui_ImplVulkan_RenderDrawData() to be consistent with other bindings.
//  2018-06-08: Misc: Extracted imgui_impl_vulkan.cpp/.h away from the old combined GLFW+Vulkan example.
//  2018-06-08: Vulkan: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-03-03: Vulkan: Various refactor, created a couple of ImGui_ImplVulkanH_XXX helper that the example can use and that viewport support will use.
//  2018-03-01: Vulkan: Renamed ImGui_ImplVulkan_Init_Info to ImGui_ImplVulkan_InitInfo and fields to match more closely Vulkan terminology.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback, ImGui_ImplVulkan_Render() calls ImGui_ImplVulkan_RenderDrawData() itself.
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2017-05-15: Vulkan: Fix scissor offset being negative. Fix new Vulkan validation warnings. Set required depth member for buffer image copy.
//  2016-11-13: Vulkan: Fix validation layer warnings and errors and redeclare gl_PerVertex.
//  2016-10-18: Vulkan: Add location decorators & change to use structs as in/out in glsl, update embedded spv (produced with glslangValidator -x). Null the released resources.
//  2016-08-27: Vulkan: Fix Vulkan example for use when a depth buffer is active.

#include "imgui/include/imgui.h"
#include "imgui/include/imgui_impl_vulkan.h"
#include "Graphics/Vulkan/VulkanInstanceProcedures.h"
#include <stdio.h>

// Reusable buffers used for rendering 1 current in-flight frame, for ImGui_ImplVulkan_RenderDrawData()
// [Please zero-clear before use!]
struct ImGui_ImplVulkanH_FrameRenderBuffers
{
    Pu::DeviceMemoryHndl      VertexBufferMemory;
    Pu::DeviceMemoryHndl      IndexBufferMemory;
    Pu::DeviceSize            VertexBufferSize;
    Pu::DeviceSize            IndexBufferSize;
    Pu::BufferHndl            VertexBuffer;
    Pu::BufferHndl            IndexBuffer;
};

// Each viewport will hold 1 ImGui_ImplVulkanH_WindowRenderBuffers
// [Please zero-clear before use!]
struct ImGui_ImplVulkanH_WindowRenderBuffers
{
    uint32_t            Index;
    uint32_t            Count;
    ImGui_ImplVulkanH_FrameRenderBuffers*   FrameRenderBuffers;
};

// Vulkan data
static ImGui_ImplVulkan_InitInfo g_VulkanInitInfo = {};
static Pu::RenderPassHndl             g_RenderPass = nullptr;
static Pu::DeviceSize                 g_BufferMemoryAlignment = 256;
static Pu::PipelineCreateFlags         g_PipelineCreateFlags = Pu::PipelineCreateFlags::None;
static Pu::DescriptorSetLayoutHndl    g_DescriptorSetLayout = nullptr;
static Pu::PipelineLayoutHndl         g_PipelineLayout = nullptr;
static Pu::DescriptorSetHndl          g_DescriptorSet = nullptr;
static Pu::PipelineHndl               g_Pipeline = nullptr;

// Font data
static Pu::SamplerHndl                g_FontSampler = nullptr;
static Pu::DeviceMemoryHndl           g_FontMemory = nullptr;
static Pu::ImageHndl                  g_FontImage = nullptr;
static Pu::ImageViewHndl              g_FontView = nullptr;
static Pu::DeviceMemoryHndl           g_UploadBufferMemory = nullptr;
static Pu::BufferHndl                 g_UploadBuffer = nullptr;

// Render buffers
static ImGui_ImplVulkanH_WindowRenderBuffers    g_MainWindowRenderBuffers;

// Forward Declarations
bool ImGui_ImplVulkan_CreateDeviceObjects();
void ImGui_ImplVulkan_DestroyDeviceObjects();
void ImGui_ImplVulkanH_DestroyFrame(Pu::DeviceHndl device, ImGui_ImplVulkanH_Frame* fd, const Pu::AllocationCallbacks* allocator);
void ImGui_ImplVulkanH_DestroyFrameSemaphores(Pu::DeviceHndl device, ImGui_ImplVulkanH_FrameSemaphores* fsd, const Pu::AllocationCallbacks* allocator);
void ImGui_ImplVulkanH_DestroyFrameRenderBuffers(Pu::DeviceHndl device, ImGui_ImplVulkanH_FrameRenderBuffers* buffers, const Pu::AllocationCallbacks* allocator);
void ImGui_ImplVulkanH_DestroyWindowRenderBuffers(Pu::DeviceHndl device, ImGui_ImplVulkanH_WindowRenderBuffers* buffers, const Pu::AllocationCallbacks* allocator);
void ImGui_ImplVulkanH_CreateWindowSwapChain(Pu::PhysicalDeviceHndl physical_device, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wd, const Pu::AllocationCallbacks* allocator, int w, int h, uint32_t min_image_count);
void ImGui_ImplVulkanH_CreateWindowCommandBuffers(Pu::PhysicalDeviceHndl physical_device, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wd, uint32_t queue_family, const Pu::AllocationCallbacks* allocator);

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
    Out.Color = aColor;
    Out.UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static uint32_t __glsl_shader_vert_spv[] =
{
    0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
    0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
    0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
    0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
    0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
    0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
    0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
    0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
    0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
    0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
    0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
    0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
    0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
    0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
    0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
    0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
    0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
    0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
    0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
    0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
    0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
    0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
    0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
    0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
    0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
    0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
    0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
    0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
    0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
    0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
    0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
    0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
    0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
    0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
    0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
    0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
    0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
    0x0000002d,0x0000002c,0x000100fd,0x00010038
};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
    fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static uint32_t __glsl_shader_frag_spv[] =
{
    0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
    0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
    0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
    0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
    0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
    0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
    0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
    0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
    0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
    0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
    0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
    0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
    0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
    0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
    0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
    0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
    0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
    0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
    0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
    0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
    0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
    0x00010038
};

//-----------------------------------------------------------------------------
// FUNCTIONS
//-----------------------------------------------------------------------------

static uint32_t ImGui_ImplVulkan_MemoryType(Pu::MemoryPropertyFlags properties, uint32_t type_bits)
{
    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    Pu::PhysicalDeviceMemoryProperties prop;
    Pu::vkGetPhysicalDeviceMemoryProperties(v->PhysicalDevice, &prop);
    for (uint32_t i = 0; i < prop.MemoryTypeCount; i++)
        if (Pu::_CrtEnumBitAnd(prop.MemoryTypes[i].PropertyFlags, properties) == properties && type_bits & (1<<i))
            return i;
    return 0xFFFFFFFF; // Unable to find memoryType
}

static void check_vk_result(Pu::VkApiResult err)
{
    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    if (v->CheckVkResultFn)
        v->CheckVkResultFn(err);
}

static void CreateOrResizeBuffer(Pu::BufferHndl& buffer, Pu::DeviceMemoryHndl& buffer_memory, Pu::DeviceSize& p_buffer_size, size_t new_size, Pu::BufferUsageFlags usage)
{
    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    Pu::VkApiResult err;
    if (buffer != nullptr)
		Pu::vkDestroyBuffer(v->Device, buffer, v->Allocator);
    if (buffer_memory != nullptr)
		Pu::vkFreeMemory(v->Device, buffer_memory, v->Allocator);

    Pu::DeviceSize vertex_buffer_size_aligned = ((new_size - 1) / g_BufferMemoryAlignment + 1) * g_BufferMemoryAlignment;
    Pu::BufferCreateInfo buffer_info = {};
    buffer_info.Size = vertex_buffer_size_aligned;
    buffer_info.Usage = usage;
    buffer_info.SharingMode = Pu::SharingMode::Exclusive;
    err = Pu::vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &buffer);
    check_vk_result(err);

    Pu::MemoryRequirements req;
	Pu::vkGetBufferMemoryRequirements(v->Device, buffer, &req);
    g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.Alignment) ? g_BufferMemoryAlignment : req.Alignment;
    Pu::MemoryAllocateInfo alloc_info = {};
    alloc_info.AllocationSize = req.Size;
    alloc_info.MemoryTypeIndex = ImGui_ImplVulkan_MemoryType(Pu::MemoryPropertyFlags::HostVisible, req.MemoryTypeBits);
    err = Pu::vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &buffer_memory);
    check_vk_result(err);

    err = Pu::vkBindBufferMemory(v->Device, buffer, buffer_memory, 0);
    check_vk_result(err);
    p_buffer_size = new_size;
}

static void ImGui_ImplVulkan_SetupRenderState(ImDrawData* draw_data, Pu::CommandBufferHndl command_buffer, ImGui_ImplVulkanH_FrameRenderBuffers* rb, int fb_width, int fb_height)
{
    // Bind pipeline and descriptor sets:
    {
        Pu::vkCmdBindPipeline(command_buffer, Pu::PipelineBindPoint::Graphics, g_Pipeline);
        Pu::DescriptorSetHndl desc_set[1] = { g_DescriptorSet };
        Pu::vkCmdBindDescriptorSets(command_buffer, Pu::PipelineBindPoint::Graphics, g_PipelineLayout, 0, 1, desc_set, 0, NULL);
    }

    // Bind Vertex And Index Buffer:
    {
        Pu::BufferHndl vertex_buffers[1] = { rb->VertexBuffer };
        Pu::DeviceSize vertex_offset[1] = { 0 };
        Pu::vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, vertex_offset);
        Pu::vkCmdBindIndexBuffer(command_buffer, rb->IndexBuffer, 0, sizeof(ImDrawIdx) == 2 ? Pu::IndexType::UInt16 : Pu::IndexType::UInt32);
    }

    // Setup viewport:
    {
        Pu::Viewport viewport;
        viewport.X = 0;
        viewport.Y = 0;
        viewport.Width = (float)fb_width;
        viewport.Height = (float)fb_height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
		Pu::vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    }

    // Setup scale and translation:
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is typically (0,0) for single viewport apps.
    {
        float scale[2];
        scale[0] = 2.0f / draw_data->DisplaySize.x;
        scale[1] = 2.0f / draw_data->DisplaySize.y;
        float translate[2];
        translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
        translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
        Pu::vkCmdPushConstants(command_buffer, g_PipelineLayout, Pu::ShaderStageFlags::Vertex, sizeof(float) * 0, sizeof(float) * 2, scale);
        Pu::vkCmdPushConstants(command_buffer, g_PipelineLayout, Pu::ShaderStageFlags::Vertex, sizeof(float) * 2, sizeof(float) * 2, translate);
    }
}

// Render function
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
void ImGui_ImplVulkan_RenderDrawData(ImDrawData* draw_data, Pu::CommandBufferHndl command_buffer)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0 || draw_data->TotalVtxCount == 0)
        return;

    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;

    // Allocate array to store enough vertex/index buffers
    ImGui_ImplVulkanH_WindowRenderBuffers* wrb = &g_MainWindowRenderBuffers;
    if (wrb->FrameRenderBuffers == NULL)
    {
        wrb->Index = 0;
        wrb->Count = v->ImageCount;
        wrb->FrameRenderBuffers = (ImGui_ImplVulkanH_FrameRenderBuffers*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_FrameRenderBuffers) * wrb->Count);
        memset(wrb->FrameRenderBuffers, 0, sizeof(ImGui_ImplVulkanH_FrameRenderBuffers) * wrb->Count);
    }
    IM_ASSERT(wrb->Count == v->ImageCount);
    wrb->Index = (wrb->Index + 1) % wrb->Count;
    ImGui_ImplVulkanH_FrameRenderBuffers* rb = &wrb->FrameRenderBuffers[wrb->Index];

    Pu::VkApiResult err;

    // Create or resize the vertex/index buffers
    size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
    size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
    if (rb->VertexBuffer == nullptr || rb->VertexBufferSize < vertex_size)
        CreateOrResizeBuffer(rb->VertexBuffer, rb->VertexBufferMemory, rb->VertexBufferSize, vertex_size, Pu::BufferUsageFlags::VertexBuffer);
    if (rb->IndexBuffer == nullptr || rb->IndexBufferSize < index_size)
        CreateOrResizeBuffer(rb->IndexBuffer, rb->IndexBufferMemory, rb->IndexBufferSize, index_size, Pu::BufferUsageFlags::IndexBuffer);

    // Upload vertex/index data into a single contiguous GPU buffer
    {
        ImDrawVert* vtx_dst = NULL;
        ImDrawIdx* idx_dst = NULL;
        err = Pu::vkMapMemory(v->Device, rb->VertexBufferMemory, 0, vertex_size, 0, (void**)(&vtx_dst));
        check_vk_result(err);
        err = Pu::vkMapMemory(v->Device, rb->IndexBufferMemory, 0, index_size, 0, (void**)(&idx_dst));
        check_vk_result(err);
        for (int n = 0; n < draw_data->CmdListsCount; n++)
        {
            const ImDrawList* cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
        Pu::MappedMemoryRange range[2] = {};
        range[0].Memory = rb->VertexBufferMemory;
        range[0].Size = Pu::WholeSize;
        range[1].Memory = rb->IndexBufferMemory;
        range[1].Size = Pu::WholeSize;
        err = Pu::vkFlushMappedMemoryRanges(v->Device, 2, range);
        check_vk_result(err);
		Pu::vkUnmapMemory(v->Device, rb->VertexBufferMemory);
		Pu::vkUnmapMemory(v->Device, rb->IndexBufferMemory);
    }

    // Setup desired Vulkan state
    ImGui_ImplVulkan_SetupRenderState(draw_data, command_buffer, rb, fb_width, fb_height);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplVulkan_SetupRenderState(draw_data, command_buffer, rb, fb_width, fb_height);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Negative offsets are illegal for vkCmdSetScissor
                    if (clip_rect.x < 0.0f)
                        clip_rect.x = 0.0f;
                    if (clip_rect.y < 0.0f)
                        clip_rect.y = 0.0f;

                    // Apply scissor/clipping rectangle
                    Pu::Rect2D scissor;
                    scissor.Offset.X = (int32_t)(clip_rect.x);
                    scissor.Offset.Y = (int32_t)(clip_rect.y);
                    scissor.Extent.Width = (uint32_t)(clip_rect.z - clip_rect.x);
                    scissor.Extent.Height = (uint32_t)(clip_rect.w - clip_rect.y);
					Pu::vkCmdSetScissor(command_buffer, 0, 1, &scissor);

                    // Draw
					Pu::vkCmdDrawIndexed(command_buffer, pcmd->ElemCount, 1, idx_offset, vtx_offset, 0);
                }
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

bool ImGui_ImplVulkan_CreateFontsTexture(Pu::CommandBufferHndl command_buffer)
{
    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t upload_size = width*height*4*sizeof(char);

    Pu::VkApiResult err;

    // Create the Image:
    {
        Pu::ImageCreateInfo info = {};
        info.ImageType = Pu::ImageType::Image2D;
		info.Format = Pu::Format::R8G8B8A8_UNORM;
        info.Extent.Width = width;
        info.Extent.Height = height;
        info.Extent.Depth = 1;
        info.MipLevels = 1;
        info.ArrayLayers = 1;
        info.Samples = Pu::SampleCountFlags::Pixel1Bit;
        info.Tiling = Pu::ImageTiling::Optimal;
        info.Usage = Pu::ImageUsageFlags::Sampled | Pu::ImageUsageFlags::TransferDst;
        info.SharingMode = Pu::SharingMode::Exclusive;
        info.InitialLayout = Pu::ImageLayout::Undefined;
        err = Pu::vkCreateImage(v->Device, &info, v->Allocator, &g_FontImage);
        check_vk_result(err);
        Pu::MemoryRequirements req;
		Pu::vkGetImageMemoryRequirements(v->Device, g_FontImage, &req);
        Pu::MemoryAllocateInfo alloc_info = {};
        alloc_info.AllocationSize = req.Size;
        alloc_info.MemoryTypeIndex = ImGui_ImplVulkan_MemoryType(Pu::MemoryPropertyFlags::DeviceLocal, req.MemoryTypeBits);
        err = Pu::vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &g_FontMemory);
        check_vk_result(err);
        err = Pu::vkBindImageMemory(v->Device, g_FontImage, g_FontMemory, 0);
        check_vk_result(err);
    }

    // Create the Image View:
    {
        Pu::ImageViewCreateInfo info = {};
        info.Image = g_FontImage;
        info.ViewType = Pu::ImageViewType::Image2D;
        info.Format = Pu::Format::R8G8B8A8_UNORM;
        info.SubresourceRange.AspectMask = Pu::ImageAspectFlags::Color;
        info.SubresourceRange.LevelCount = 1;
        info.SubresourceRange.LayerCount = 1;
        err = Pu::vkCreateImageView(v->Device, &info, v->Allocator, &g_FontView);
        check_vk_result(err);
    }

    // Update the Descriptor Set:
    {
        Pu::DescriptorImageInfo desc_image[1] = {};
        desc_image[0].Sampler = g_FontSampler;
        desc_image[0].ImageView = g_FontView;
        desc_image[0].ImageLayout = Pu::ImageLayout::ShaderReadOnlyOptimal;
        Pu::WriteDescriptorSet write_desc[1] = {};
        write_desc[0].DstSet = g_DescriptorSet;
        write_desc[0].DescriptorCount = 1;
        write_desc[0].DescriptorType = Pu::DescriptorType::CombinedImageSampler;
        write_desc[0].ImageInfo = desc_image;
		Pu::vkUpdateDescriptorSets(v->Device, 1, write_desc, 0, NULL);
    }

    // Create the Upload Buffer:
    {
        Pu::BufferCreateInfo buffer_info = {};
        buffer_info.Size = upload_size;
        buffer_info.Usage = Pu::BufferUsageFlags::TransferSrc;
        buffer_info.SharingMode = Pu::SharingMode::Exclusive;
        err = Pu::vkCreateBuffer(v->Device, &buffer_info, v->Allocator, &g_UploadBuffer);
        check_vk_result(err);
        Pu::MemoryRequirements req;
		Pu::vkGetBufferMemoryRequirements(v->Device, g_UploadBuffer, &req);
        g_BufferMemoryAlignment = (g_BufferMemoryAlignment > req.Alignment) ? g_BufferMemoryAlignment : req.Alignment;
        Pu::MemoryAllocateInfo alloc_info = {};
        alloc_info.AllocationSize = req.Size;
        alloc_info.MemoryTypeIndex = ImGui_ImplVulkan_MemoryType(Pu::MemoryPropertyFlags::HostVisible, req.MemoryTypeBits);
        err = Pu::vkAllocateMemory(v->Device, &alloc_info, v->Allocator, &g_UploadBufferMemory);
        check_vk_result(err);
        err = Pu::vkBindBufferMemory(v->Device, g_UploadBuffer, g_UploadBufferMemory, 0);
        check_vk_result(err);
    }

    // Upload to Buffer:
    {
        char* map = NULL;
        err = Pu::vkMapMemory(v->Device, g_UploadBufferMemory, 0, upload_size, 0, (void**)(&map));
        check_vk_result(err);
        memcpy(map, pixels, upload_size);
        Pu::MappedMemoryRange range[1] = {};
        range[0].Memory = g_UploadBufferMemory;
        range[0].Size = upload_size;
        err = Pu::vkFlushMappedMemoryRanges(v->Device, 1, range);
        check_vk_result(err);
		Pu::vkUnmapMemory(v->Device, g_UploadBufferMemory);
    }

    // Copy to Image:
    {
        Pu::ImageMemoryBarrier copy_barrier[1] = {};
        copy_barrier[0].DstAccessMask = Pu::AccessFlags::TransferWrite;
        copy_barrier[0].OldLayout = Pu::ImageLayout::Undefined;
        copy_barrier[0].NewLayout = Pu::ImageLayout::TransferDstOptimal;
        copy_barrier[0].SrcQueueFamilyIndex = Pu::QueueFamilyIgnored;
        copy_barrier[0].DstQueueFamilyIndex = Pu::QueueFamilyIgnored;
        copy_barrier[0].Image = g_FontImage;
        copy_barrier[0].SubresourceRange.AspectMask = Pu::ImageAspectFlags::Color;
        copy_barrier[0].SubresourceRange.LevelCount = 1;
        copy_barrier[0].SubresourceRange.LayerCount = 1;
		Pu::vkCmdPipelineBarrier(command_buffer, Pu::PipelineStageFlags::Host, Pu::PipelineStageFlags::Transfer, Pu::DependencyFlags::None, 0, NULL, 0, NULL, 1, copy_barrier);

        Pu::BufferImageCopy region = {};
        region.ImageSubresource.AspectMask = Pu::ImageAspectFlags::Color;
        region.ImageSubresource.LayerCount = 1;
        region.ImageExtent.Width = width;
        region.ImageExtent.Height = height;
        region.ImageExtent.Depth = 1;
		Pu::vkCmdCopyBufferToImage(command_buffer, g_UploadBuffer, g_FontImage, Pu::ImageLayout::TransferDstOptimal, 1, &region);

        Pu::ImageMemoryBarrier use_barrier[1] = {};
        use_barrier[0].SrcAccessMask = Pu::AccessFlags::TransferWrite;
        use_barrier[0].DstAccessMask = Pu::AccessFlags::ShaderRead;
        use_barrier[0].OldLayout = Pu::ImageLayout::TransferDstOptimal;
        use_barrier[0].NewLayout = Pu::ImageLayout::ShaderReadOnlyOptimal;
        use_barrier[0].SrcQueueFamilyIndex = Pu::QueueFamilyIgnored;
        use_barrier[0].DstQueueFamilyIndex = Pu::QueueFamilyIgnored;
        use_barrier[0].Image = g_FontImage;
        use_barrier[0].SubresourceRange.AspectMask = Pu::ImageAspectFlags::Color;
        use_barrier[0].SubresourceRange.LevelCount = 1;
        use_barrier[0].SubresourceRange.LayerCount = 1;
		Pu::vkCmdPipelineBarrier(command_buffer, Pu::PipelineStageFlags::Transfer, Pu::PipelineStageFlags::FragmentShader, Pu::DependencyFlags::None, 0, NULL, 0, NULL, 1, use_barrier);
    }

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontImage;

    return true;
}

bool ImGui_ImplVulkan_CreateDeviceObjects()
{
    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    Pu::VkApiResult err;
    Pu::ShaderModuleHndl vert_module;
    Pu::ShaderModuleHndl frag_module;

    // Create The Shader Modules:
    {
        Pu::ShaderModuleCreateInfo vert_info = {};
        vert_info.CodeSize = sizeof(__glsl_shader_vert_spv);
        vert_info.Code = (uint32_t*)__glsl_shader_vert_spv;
        err = Pu::vkCreateShaderModule(v->Device, &vert_info, v->Allocator, &vert_module);
        check_vk_result(err);
        Pu::ShaderModuleCreateInfo frag_info = {};
        frag_info.CodeSize = sizeof(__glsl_shader_frag_spv);
        frag_info.Code = (uint32_t*)__glsl_shader_frag_spv;
        err = Pu::vkCreateShaderModule(v->Device, &frag_info, v->Allocator, &frag_module);
        check_vk_result(err);
    }

    if (!g_FontSampler)
    {
        Pu::SamplerCreateInfo info = {};
        info.MagFilter = Pu::Filter::Linear;
        info.MinFilter = Pu::Filter::Linear;
        info.MipmapMode = Pu::SamplerMipmapMode::Linear;
        info.AddressModeU = Pu::SamplerAddressMode::Repeat;
        info.AddressModeV = Pu::SamplerAddressMode::Repeat;
        info.AddressModeW = Pu::SamplerAddressMode::Repeat;
        info.MinLoD = -1000;
        info.MaxLoD = 1000;
        info.MaxAnisotropy = 1.0f;
        err = Pu::vkCreateSampler(v->Device, &info, v->Allocator, &g_FontSampler);
        check_vk_result(err);
    }

    if (!g_DescriptorSetLayout)
    {
        Pu::SamplerHndl sampler[1] = {g_FontSampler};
        Pu::DescriptorSetLayoutBinding binding[1] = {};
        binding[0].DescriptorType = Pu::DescriptorType::CombinedImageSampler;
        binding[0].DescriptorCount = 1;
        binding[0].StageFlags = Pu::ShaderStageFlags::Fragment;
        binding[0].ImmutableSamplers = sampler;
        Pu::DescriptorSetLayoutCreateInfo info = {};
        info.BindingCount = 1;
        info.Bindings = binding;
        err = Pu::vkCreateDescriptorSetLayout(v->Device, &info, v->Allocator, &g_DescriptorSetLayout);
        check_vk_result(err);
    }

    // Create Descriptor Set:
    {
        Pu::DescriptorSetAllocateInfo alloc_info = {};
        alloc_info.DescriptorPool = v->DescriptorPool;
        alloc_info.DescriptorSetCount = 1;
        alloc_info.SetLayouts = &g_DescriptorSetLayout;
        err = Pu::vkAllocateDescriptorSets(v->Device, &alloc_info, &g_DescriptorSet);
        check_vk_result(err);
    }

    if (!g_PipelineLayout)
    {
        // Constants: we are using 'vec2 offset' and 'vec2 scale' instead of a full 3d projection matrix
        Pu::PushConstantRange push_constants[1] = {};
        push_constants[0].StageFlags = Pu::ShaderStageFlags::Vertex;
        push_constants[0].Offset = sizeof(float) * 0;
        push_constants[0].Size = sizeof(float) * 4;
        Pu::DescriptorSetLayoutHndl set_layout[1] = { g_DescriptorSetLayout };
        Pu::PipelineLayoutCreateInfo layout_info = {};
        layout_info.SetLayoutCount = 1;
        layout_info.SetLayouts = set_layout;
        layout_info.PushConstantRangeCount = 1;
        layout_info.PushConstantRanges = push_constants;
        err = Pu::vkCreatePipelineLayout(v->Device, &layout_info, v->Allocator, &g_PipelineLayout);
        check_vk_result(err);
    }

    Pu::PipelineShaderStageCreateInfo stage[2] = {};
    stage[0].Stage = Pu::ShaderStageFlags::Vertex;
    stage[0].Module = vert_module;
    stage[0].Name = "main";
    stage[1].Stage = Pu::ShaderStageFlags::Fragment;
    stage[1].Module = frag_module;
    stage[1].Name = "main";

    Pu::VertexInputBindingDescription binding_desc[1] = {};
    binding_desc[0].Stride = sizeof(ImDrawVert);
    binding_desc[0].InputRate = Pu::VertexInputRate::Vertex;

    Pu::VertexInputAttributeDescription attribute_desc[3] = {};
    attribute_desc[0].Location = 0;
    attribute_desc[0].Binding = binding_desc[0].Binding;
    attribute_desc[0].Format = Pu::Format::R32G32_SFLOAT;
    attribute_desc[0].Offset = IM_OFFSETOF(ImDrawVert, pos);
    attribute_desc[1].Location = 1;
    attribute_desc[1].Binding = binding_desc[0].Binding;
    attribute_desc[1].Format = Pu::Format::R32G32_SFLOAT;
    attribute_desc[1].Offset = IM_OFFSETOF(ImDrawVert, uv);
    attribute_desc[2].Location = 2;
    attribute_desc[2].Binding = binding_desc[0].Binding;
    attribute_desc[2].Format = Pu::Format::R8G8B8A8_UNORM;
    attribute_desc[2].Offset = IM_OFFSETOF(ImDrawVert, col);

    Pu::PipelineVertexInputStateCreateInfo vertex_info = {};
    vertex_info.VertexBindingDescriptionCount = 1;
    vertex_info.VertexBindingDescriptions = binding_desc;
    vertex_info.VertexAttributeDescriptionCount = 3;
    vertex_info.VertexAttributeDescriptions = attribute_desc;

    Pu::PipelineInputAssemblyStateCreateInfo ia_info = {};
    ia_info.Topology = Pu::PrimitiveTopology::TriangleList;

    Pu::PipelineViewportStateCreateInfo viewport_info = {};
    viewport_info.ViewportCount = 1;
    viewport_info.ScissorCount = 1;

    Pu::PipelineRasterizationStateCreateInfo raster_info = {};
    raster_info.PolygonMode = Pu::PolygonMode::Fill;
    raster_info.CullMode = Pu::CullModeFlags::None;
    raster_info.FrontFace = Pu::FrontFace::CounterClockwise;
    raster_info.LineWidth = 1.0f;

    Pu::PipelineMultisampleStateCreateInfo ms_info = {};
    ms_info.RasterizationSamples = Pu::SampleCountFlags::Pixel1Bit;

    Pu::PipelineColorBlendAttachmentState color_attachment[1] = {};
    color_attachment[0].BlendEnable = true;
    color_attachment[0].SrcColorBlendFactor = Pu::BlendFactor::SrcAlpha;
    color_attachment[0].DstColorBlendFactor = Pu::BlendFactor::ISrcAlpha;
    color_attachment[0].ColorBlendOp = Pu::BlendOp::Add;
    color_attachment[0].SrcAlphaBlendFactor = Pu::BlendFactor::ISrcAlpha;
    color_attachment[0].DstAlphaBlendFactor = Pu::BlendFactor::Zero;
    color_attachment[0].AlphaBlendOp = Pu::BlendOp::Add;
    color_attachment[0].ColorWriteMask = Pu::ColorComponentFlags::RGBA;

    Pu::PipelineDepthStencilStateCreateInfo depth_info = {};

    Pu::PipelineColorBlendStateCreateInfo blend_info = {};
    blend_info.AttachmentCount = 1;
    blend_info.Attachments = color_attachment;

    Pu::DynamicState dynamic_states[2] = { Pu::DynamicState::ViewPort, Pu::DynamicState::Scissor };
    Pu::PipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.DynamicStateCount = (uint32_t)IM_ARRAYSIZE(dynamic_states);
    dynamic_state.DynamicStates = dynamic_states;

    Pu::GraphicsPipelineCreateInfo info = {};
    info.Flags = g_PipelineCreateFlags;
    info.StageCount = 2;
    info.Stages = stage;
    info.VertexInputState = &vertex_info;
    info.InputAssemblyState = &ia_info;
    info.ViewportState = &viewport_info;
    info.RasterizationState = &raster_info;
    info.MultisampleState = &ms_info;
    info.DepthStencilState = &depth_info;
    info.ColorBlendState = &blend_info;
    info.DynamicState = &dynamic_state;
    info.Layout = g_PipelineLayout;
    info.Renderpass = g_RenderPass;
    err = Pu::vkCreateGraphicsPipelines(v->Device, v->PipelineCache, 1, &info, v->Allocator, &g_Pipeline);
    check_vk_result(err);

	Pu::vkDestroyShaderModule(v->Device, vert_module, v->Allocator);
	Pu::vkDestroyShaderModule(v->Device, frag_module, v->Allocator);

    return true;
}

void    ImGui_ImplVulkan_DestroyFontUploadObjects()
{
    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    if (g_UploadBuffer)
    {
		Pu::vkDestroyBuffer(v->Device, g_UploadBuffer, v->Allocator);
        g_UploadBuffer = nullptr;
    }
    if (g_UploadBufferMemory)
    {
		Pu::vkFreeMemory(v->Device, g_UploadBufferMemory, v->Allocator);
        g_UploadBufferMemory = nullptr;
    }
}

void    ImGui_ImplVulkan_DestroyDeviceObjects()
{
    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    ImGui_ImplVulkanH_DestroyWindowRenderBuffers(v->Device, &g_MainWindowRenderBuffers, v->Allocator);
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    if (g_FontView)             { Pu::vkDestroyImageView(v->Device, g_FontView, v->Allocator); g_FontView = nullptr; }
    if (g_FontImage)            { Pu::vkDestroyImage(v->Device, g_FontImage, v->Allocator); g_FontImage = nullptr; }
    if (g_FontMemory)           { Pu::vkFreeMemory(v->Device, g_FontMemory, v->Allocator); g_FontMemory = nullptr; }
    if (g_FontSampler)          { Pu::vkDestroySampler(v->Device, g_FontSampler, v->Allocator); g_FontSampler = nullptr; }
    if (g_DescriptorSetLayout)  { Pu::vkDestroyDescriptorSetLayout(v->Device, g_DescriptorSetLayout, v->Allocator); g_DescriptorSetLayout = nullptr; }
    if (g_PipelineLayout)       { Pu::vkDestroyPipelineLayout(v->Device, g_PipelineLayout, v->Allocator); g_PipelineLayout = nullptr; }
    if (g_Pipeline)             { Pu::vkDestroyPipeline(v->Device, g_Pipeline, v->Allocator); g_Pipeline = nullptr; }
}

bool    ImGui_ImplVulkan_Init(ImGui_ImplVulkan_InitInfo* info, Pu::RenderPassHndl render_pass)
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_vulkan";

    IM_ASSERT(info->Instance != nullptr);
    IM_ASSERT(info->PhysicalDevice != nullptr);
    IM_ASSERT(info->Device != nullptr);
    IM_ASSERT(info->Queue != nullptr);
    IM_ASSERT(info->DescriptorPool != nullptr);
    IM_ASSERT(info->MinImageCount >= 2);
    IM_ASSERT(info->ImageCount >= info->MinImageCount);
    IM_ASSERT(render_pass != nullptr);

    g_VulkanInitInfo = *info;
    g_RenderPass = render_pass;
    ImGui_ImplVulkan_CreateDeviceObjects();

    return true;
}

void ImGui_ImplVulkan_Shutdown()
{
    ImGui_ImplVulkan_DestroyDeviceObjects();
}

void ImGui_ImplVulkan_NewFrame()
{
}

void ImGui_ImplVulkan_SetMinImageCount(uint32_t min_image_count)
{
    IM_ASSERT(min_image_count >= 2);
    if (g_VulkanInitInfo.MinImageCount == min_image_count)
        return;

    ImGui_ImplVulkan_InitInfo* v = &g_VulkanInitInfo;
    Pu::VkApiResult err = Pu::vkDeviceWaitIdle(v->Device);
    check_vk_result(err);
    ImGui_ImplVulkanH_DestroyWindowRenderBuffers(v->Device, &g_MainWindowRenderBuffers, v->Allocator);
    g_VulkanInitInfo.MinImageCount = min_image_count;
}


//-------------------------------------------------------------------------
// Internal / Miscellaneous Vulkan Helpers
// (Used by example's main.cpp. Used by multi-viewport features. PROBABLY NOT used by your own app.)
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
Pu::SurfaceFormat ImGui_ImplVulkanH_SelectSurfaceFormat(Pu::PhysicalDeviceHndl physical_device, Pu::SurfaceHndl surface, const Pu::Format* request_formats, int request_formats_count, Pu::ColorSpace request_color_space)
{
    IM_ASSERT(request_formats != NULL);
    IM_ASSERT(request_formats_count > 0);

    // Per Spec Format and View Format are expected to be the same unless VK_IMAGE_CREATE_MUTABLE_BIT was set at image creation
    // Assuming that the default behavior is without setting this bit, there is no need for separate Swapchain image and image view format
    // Additionally several new color spaces were introduced with Vulkan Spec v1.0.40,
    // hence we must make sure that a format with the mostly available color space, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, is found and used.
    uint32_t avail_count;
	Pu::vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, NULL);
    ImVector<Pu::SurfaceFormat> avail_format;
    avail_format.resize((int)avail_count);
	Pu::vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &avail_count, avail_format.Data);

    // First check if only one format, VK_FORMAT_UNDEFINED, is available, which would imply that any format is available
    if (avail_count == 1)
    {
        if (avail_format[0].Format == Pu::Format::Undefined)
        {
            Pu::SurfaceFormat ret;
            ret.Format = request_formats[0];
            ret.ColorSpace = request_color_space;
            return ret;
        }
        else
        {
            // No point in searching another format
            return avail_format[0];
        }
    }
    else
    {
        // Request several formats, the first found will be used
        for (int request_i = 0; request_i < request_formats_count; request_i++)
            for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
                if (avail_format[avail_i].Format == request_formats[request_i] && avail_format[avail_i].ColorSpace == request_color_space)
                    return avail_format[avail_i];

        // If none of the requested image formats could be found, use the first available
        return avail_format[0];
    }
}

Pu::PresentMode ImGui_ImplVulkanH_SelectPresentMode(Pu::PhysicalDeviceHndl physical_device, Pu::SurfaceHndl surface, const Pu::PresentMode* request_modes, int request_modes_count)
{
    IM_ASSERT(request_modes != NULL);
    IM_ASSERT(request_modes_count > 0);

    // Request a certain mode and confirm that it is available. If not use VK_PRESENT_MODE_FIFO_KHR which is mandatory
    uint32_t avail_count = 0;
	Pu::vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &avail_count, NULL);
    ImVector<Pu::PresentMode> avail_modes;
    avail_modes.resize((int)avail_count);
	Pu::vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &avail_count, avail_modes.Data);
    //for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
    //    printf("[vulkan] avail_modes[%d] = %d\n", avail_i, avail_modes[avail_i]);

    for (int request_i = 0; request_i < request_modes_count; request_i++)
        for (uint32_t avail_i = 0; avail_i < avail_count; avail_i++)
            if (request_modes[request_i] == avail_modes[avail_i])
                return request_modes[request_i];

    return Pu::PresentMode::FiFo; // Always available
}

void ImGui_ImplVulkanH_CreateWindowCommandBuffers(Pu::PhysicalDeviceHndl physical_device, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wd, uint32_t queue_family, const Pu::AllocationCallbacks* allocator)
{
    IM_ASSERT(physical_device != nullptr && device != nullptr);
    (void)physical_device;
    (void)allocator;

    // Create Command Buffers
    Pu::VkApiResult err;
    for (uint32_t i = 0; i < wd->ImageCount; i++)
    {
        ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
        ImGui_ImplVulkanH_FrameSemaphores* fsd = &wd->FrameSemaphores[i];
        {
            Pu::CommandPoolCreateInfo info = {};
            info.Flags = Pu::CommandPoolCreateFlags::ResetCommandBuffer;
            info.QueueFamilyIndex = queue_family;
            err = Pu::vkCreateCommandPool(device, &info, allocator, &fd->CommandPool);
            check_vk_result(err);
        }
        {
            Pu::CommandBufferAllocateInfo info = {};
            info.CommandPool = fd->CommandPool;
            info.Level = Pu::CommandBufferLevel::Primary;
            info.CommandBufferCount = 1;
            err = Pu::vkAllocateCommandBuffers(device, &info, &fd->CommandBuffer);
            check_vk_result(err);
        }
        {
            Pu::FenceCreateInfo info = {};
            info.Flags = Pu::FenceCreateFlags::Signaled;
            err = Pu::vkCreateFence(device, &info, allocator, &fd->Fence);
            check_vk_result(err);
        }
        {
            Pu::SemaphoreCreateInfo info = {};
            err = Pu::vkCreateSemaphore(device, &info, allocator, &fsd->ImageAcquiredSemaphore);
            check_vk_result(err);
            err = Pu::vkCreateSemaphore(device, &info, allocator, &fsd->RenderCompleteSemaphore);
            check_vk_result(err);
        }
    }
}

int ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(Pu::PresentMode present_mode)
{
    if (present_mode == Pu::PresentMode::MailBox)
        return 3;
    if (present_mode == Pu::PresentMode::FiFo || present_mode == Pu::PresentMode::FiFoRelaxed)
        return 2;
    if (present_mode == Pu::PresentMode::Immediate)
        return 1;
    IM_ASSERT(0);
    return 1;
}

// Also destroy old swap chain and in-flight frames data, if any.
void ImGui_ImplVulkanH_CreateWindowSwapChain(Pu::PhysicalDeviceHndl physical_device, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wd, const Pu::AllocationCallbacks* allocator, int w, int h, uint32_t min_image_count)
{
    Pu::VkApiResult err;
    Pu::SwapchainHndl old_swapchain = wd->Swapchain;
    err = Pu::vkDeviceWaitIdle(device);
    check_vk_result(err);

    // We don't use ImGui_ImplVulkanH_DestroyWindow() because we want to preserve the old swapchain to create the new one.
    // Destroy old Framebuffer
    for (uint32_t i = 0; i < wd->ImageCount; i++)
    {
        ImGui_ImplVulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
        ImGui_ImplVulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
    }
    IM_FREE(wd->Frames);
    IM_FREE(wd->FrameSemaphores);
    wd->Frames = NULL;
    wd->FrameSemaphores = NULL;
    wd->ImageCount = 0;
    if (wd->RenderPass)
		Pu::vkDestroyRenderPass(device, wd->RenderPass, allocator);

    // If min image count was not specified, request different count of images dependent on selected present mode
    if (min_image_count == 0)
        min_image_count = ImGui_ImplVulkanH_GetMinImageCountFromPresentMode(wd->PresentMode);

    // Create Swapchain
    {
        Pu::SwapchainCreateInfo info = {};
        info.Surface = wd->Surface;
        info.MinImageCount = min_image_count;
        info.ImageFormat = wd->SurfaceFormat.Format;
        info.ImageColorSpace = wd->SurfaceFormat.ColorSpace;
        info.ImageArrayLayers = 1;
        info.ImageUsage = Pu::ImageUsageFlags::ColorAttachment;
        info.ImageSharingMode = Pu::SharingMode::Exclusive;           // Assume that graphics family == present family
        info.Transform = Pu::SurfaceTransformFlags::Identity;
        info.CompositeAlpha = Pu::CompositeAlphaFlags::Opaque;
        info.PresentMode = wd->PresentMode;
        info.Clipped = true;
        info.OldSwapChain = old_swapchain;
        Pu::SurfaceCapabilities cap;
        err = Pu::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, wd->Surface, &cap);
        check_vk_result(err);
        if (info.MinImageCount < cap.MinImageCount)
            info.MinImageCount = cap.MinImageCount;
        else if (cap.MaxImageCount != 0 && info.MinImageCount > cap.MaxImageCount)
            info.MinImageCount = cap.MaxImageCount;

        if (cap.CurrentExtent.Width == 0xffffffff)
        {
            info.ImageExtent.Width = wd->Width = w;
            info.ImageExtent.Height = wd->Height = h;
        }
        else
        {
            info.ImageExtent.Width = wd->Width = cap.CurrentExtent.Width;
            info.ImageExtent.Height = wd->Height = cap.CurrentExtent.Height;
        }
        err = Pu::vkCreateSwapchainKHR(device, &info, allocator, &wd->Swapchain);
        check_vk_result(err);
        err = Pu::vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount, NULL);
        check_vk_result(err);
        Pu::ImageHndl backbuffers[16] = {};
        IM_ASSERT(wd->ImageCount >= min_image_count);
        IM_ASSERT(wd->ImageCount < IM_ARRAYSIZE(backbuffers));
        err = Pu::vkGetSwapchainImagesKHR(device, wd->Swapchain, &wd->ImageCount, backbuffers);
        check_vk_result(err);

        IM_ASSERT(wd->Frames == NULL);
        wd->Frames = (ImGui_ImplVulkanH_Frame*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_Frame) * wd->ImageCount);
        wd->FrameSemaphores = (ImGui_ImplVulkanH_FrameSemaphores*)IM_ALLOC(sizeof(ImGui_ImplVulkanH_FrameSemaphores) * wd->ImageCount);
        memset(wd->Frames, 0, sizeof(wd->Frames[0]) * wd->ImageCount);
        memset(wd->FrameSemaphores, 0, sizeof(wd->FrameSemaphores[0]) * wd->ImageCount);
        for (uint32_t i = 0; i < wd->ImageCount; i++)
            wd->Frames[i].Backbuffer = backbuffers[i];
    }
    if (old_swapchain)
		Pu::vkDestroySwapchainKHR(device, old_swapchain, allocator);

    // Create the Render Pass
    {
        Pu::AttachmentDescription attachment = {};
        attachment.Format = wd->SurfaceFormat.Format;
        attachment.Samples = Pu::SampleCountFlags::Pixel1Bit;
        attachment.LoadOp = wd->ClearEnable ? Pu::AttachmentLoadOp::Clear : Pu::AttachmentLoadOp::DontCare;
        attachment.StoreOp = Pu::AttachmentStoreOp::Store;
        attachment.StencilLoadOp = Pu::AttachmentLoadOp::DontCare;
        attachment.StencilStoreOp = Pu::AttachmentStoreOp::DontCare;
        attachment.InitialLayout = Pu::ImageLayout::Undefined;
        attachment.FinalLayout = Pu::ImageLayout::PresentSrcKhr;
        Pu::AttachmentReference color_attachment = {};
        color_attachment.Attachment = 0;
        color_attachment.Layout = Pu::ImageLayout::ColorAttachmentOptimal;
        Pu::SubpassDescription subpass = {};
        subpass.BindPoint = Pu::PipelineBindPoint::Graphics;
        subpass.ColorAttachmentCount = 1;
        subpass.ColorAttachments = &color_attachment;
        Pu::SubpassDependency dependency = {};
        dependency.SrcSubpass = Pu::SubpassExternal;
        dependency.DstSubpass = 0;
        dependency.SrcStageMask = Pu::PipelineStageFlags::ColorAttachmentOutput;
        dependency.DstStageMask = Pu::PipelineStageFlags::ColorAttachmentOutput;
        dependency.SrcAccessMask = Pu::AccessFlags::None;
        dependency.DstAccessMask = Pu::AccessFlags::ColorAttachmentWrite;
        Pu::RenderPassCreateInfo info = {};
        info.AttachmentCount = 1;
        info.Attachments = &attachment;
        info.SubpassCount = 1;
        info.Subpasses = &subpass;
        info.DependencyCount = 1;
        info.Dependencies = &dependency;
        err = Pu::vkCreateRenderPass(device, &info, allocator, &wd->RenderPass);
        check_vk_result(err);
    }

    // Create The Image Views
    {
        Pu::ImageViewCreateInfo info = {};
        info.ViewType = Pu::ImageViewType::Image2D;
        info.Format = wd->SurfaceFormat.Format;
        info.Components.R = Pu::ComponentSwizzle::R;
        info.Components.G = Pu::ComponentSwizzle::G;
        info.Components.B = Pu::ComponentSwizzle::B;
        info.Components.A = Pu::ComponentSwizzle::A;
        Pu::ImageSubresourceRange image_range = { };
        info.SubresourceRange = image_range;
        for (uint32_t i = 0; i < wd->ImageCount; i++)
        {
            ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
            info.Image = fd->Backbuffer;
            err = Pu::vkCreateImageView(device, &info, allocator, &fd->BackbufferView);
            check_vk_result(err);
        }
    }

    // Create Framebuffer
    {
        Pu::ImageViewHndl attachment[1];
        Pu::FramebufferCreateInfo info = {};
        info.RenderPass = wd->RenderPass;
        info.AttachmentCount = 1;
        info.Attachments = attachment;
        info.Width = wd->Width;
        info.Height = wd->Height;
        info.Layers = 1;
        for (uint32_t i = 0; i < wd->ImageCount; i++)
        {
            ImGui_ImplVulkanH_Frame* fd = &wd->Frames[i];
            attachment[0] = fd->BackbufferView;
            err = Pu::vkCreateFramebuffer(device, &info, allocator, &fd->Framebuffer);
            check_vk_result(err);
        }
    }
}

void ImGui_ImplVulkanH_CreateWindow(Pu::InstanceHndl instance, Pu::PhysicalDeviceHndl physical_device, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wd, uint32_t queue_family, const Pu::AllocationCallbacks* allocator, int width, int height, uint32_t min_image_count)
{
    (void)instance;
    ImGui_ImplVulkanH_CreateWindowSwapChain(physical_device, device, wd, allocator, width, height, min_image_count);
    ImGui_ImplVulkanH_CreateWindowCommandBuffers(physical_device, device, wd, queue_family, allocator);
}

void ImGui_ImplVulkanH_DestroyWindow(Pu::InstanceHndl instance, Pu::DeviceHndl device, ImGui_ImplVulkanH_Window* wd, const Pu::AllocationCallbacks* allocator)
{
	Pu::vkDeviceWaitIdle(device); // FIXME: We could wait on the Queue if we had the queue in wd-> (otherwise VulkanH functions can't use globals)
    //vkQueueWaitIdle(g_Queue);

    for (uint32_t i = 0; i < wd->ImageCount; i++)
    {
        ImGui_ImplVulkanH_DestroyFrame(device, &wd->Frames[i], allocator);
        ImGui_ImplVulkanH_DestroyFrameSemaphores(device, &wd->FrameSemaphores[i], allocator);
    }
    IM_FREE(wd->Frames);
    IM_FREE(wd->FrameSemaphores);
    wd->Frames = NULL;
    wd->FrameSemaphores = NULL;
    Pu::vkDestroyRenderPass(device, wd->RenderPass, allocator);
    Pu::vkDestroySwapchainKHR(device, wd->Swapchain, allocator);
    Pu::vkDestroySurfaceKHR(instance, wd->Surface, allocator);

    *wd = ImGui_ImplVulkanH_Window();
}

void ImGui_ImplVulkanH_DestroyFrame(Pu::DeviceHndl device, ImGui_ImplVulkanH_Frame* fd, const Pu::AllocationCallbacks* allocator)
{
    Pu::vkDestroyFence(device, fd->Fence, allocator);
    Pu::vkFreeCommandBuffers(device, fd->CommandPool, 1, &fd->CommandBuffer);
    Pu::vkDestroyCommandPool(device, fd->CommandPool, allocator);
    fd->Fence = nullptr;
    fd->CommandBuffer = nullptr;
    fd->CommandPool = nullptr;

    Pu::vkDestroyImageView(device, fd->BackbufferView, allocator);
    Pu::vkDestroyFramebuffer(device, fd->Framebuffer, allocator);
}

void ImGui_ImplVulkanH_DestroyFrameSemaphores(Pu::DeviceHndl device, ImGui_ImplVulkanH_FrameSemaphores* fsd, const Pu::AllocationCallbacks* allocator)
{
	Pu::vkDestroySemaphore(device, fsd->ImageAcquiredSemaphore, allocator);
	Pu::vkDestroySemaphore(device, fsd->RenderCompleteSemaphore, allocator);
    fsd->ImageAcquiredSemaphore = fsd->RenderCompleteSemaphore = nullptr;
}

void ImGui_ImplVulkanH_DestroyFrameRenderBuffers(Pu::DeviceHndl device, ImGui_ImplVulkanH_FrameRenderBuffers* buffers, const Pu::AllocationCallbacks* allocator)
{
    if (buffers->VertexBuffer) { Pu::vkDestroyBuffer(device, buffers->VertexBuffer, allocator); buffers->VertexBuffer = nullptr; }
    if (buffers->VertexBufferMemory) { Pu::vkFreeMemory(device, buffers->VertexBufferMemory, allocator); buffers->VertexBufferMemory = nullptr; }
    if (buffers->IndexBuffer) { Pu::vkDestroyBuffer(device, buffers->IndexBuffer, allocator); buffers->IndexBuffer = nullptr; }
    if (buffers->IndexBufferMemory) { Pu::vkFreeMemory(device, buffers->IndexBufferMemory, allocator); buffers->IndexBufferMemory = nullptr; }
    buffers->VertexBufferSize = 0;
    buffers->IndexBufferSize = 0;
}

void ImGui_ImplVulkanH_DestroyWindowRenderBuffers(Pu::DeviceHndl device, ImGui_ImplVulkanH_WindowRenderBuffers* buffers, const Pu::AllocationCallbacks* allocator)
{
    for (uint32_t n = 0; n < buffers->Count; n++)
        ImGui_ImplVulkanH_DestroyFrameRenderBuffers(device, &buffers->FrameRenderBuffers[n], allocator);
    IM_FREE(buffers->FrameRenderBuffers);
    buffers->FrameRenderBuffers = NULL;
    buffers->Index = 0;
    buffers->Count = 0;
}