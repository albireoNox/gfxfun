#include "d3d_window.h"

D3DWindow::D3DWindow(const std::wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	Window(name, clientWidth, clientHeight, hInstance)
{
	// NOOP (for now)
}

D3DWindow::~D3DWindow()
{
	// NOOP (for now)
}
