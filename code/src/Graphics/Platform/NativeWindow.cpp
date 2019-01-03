#include "Graphics/Platform/NativeWindow.h"

Pu::NativeWindow::NativeWindow(void)
	: OnSizeChanged("NativeWindowSizeChanged"),
	OnLocationChanged("NativeWindowLocationChanged"),
	OnGainedFocus("NativeWindowGainedFocus"),
	OnLostFocus("NativeWindowLoseFocus"),
	shouldSuppressRender(false)
{}