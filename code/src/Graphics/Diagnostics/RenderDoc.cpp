#include "Graphics/Diagnostics/RenderDoc.h"
#include "Core/Diagnostics/Logging.h"
#include <renderdoc/renderdoc_app.h>

void Pu::RenderDoc::StartFrameCapture(const LogicalDevice & device)
{
	RENDERDOC_API_1_4_0 *api = GetInstance().api;
	if (api) api->StartFrameCapture(device.hndl, nullptr);
}

void Pu::RenderDoc::EndFrameCapture(const LogicalDevice & device)
{
	RENDERDOC_API_1_4_0 *api = GetInstance().api;
	if (api)api->EndFrameCapture(device.hndl, nullptr);
}

Pu::RenderDoc::RenderDoc(void)
	: loader(L"renderdoc.dll"), api(nullptr)
{
	/* Only attempt to load the API if the dll could be loaded. */
	if (loader.IsUsable())
	{
		pRENDERDOC_GetAPI getter = loader.LoadProc<pRENDERDOC_GetAPI>("RENDERDOC_GetAPI");
		const int result = getter(eRENDERDOC_API_Version_1_4_0, reinterpret_cast<void**>(&api));
		if (result != 1) Log::Error("Unable to initialize RenderDoc API!");
	}
}

Pu::RenderDoc & Pu::RenderDoc::GetInstance(void)
{
	static RenderDoc api;
	return api;
}