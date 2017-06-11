#pragma once

#include "window.h"

class D3DWindow : public Window
{
public:
	// Initializes the window. May throw.
	D3DWindow(
		const std::wstring& name,
		uint clientWidth,
		uint clientHeight,
		HINSTANCE hInstance);

	~D3DWindow();

// State
};
