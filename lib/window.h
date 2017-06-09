#pragma once

#include "common.h"
#include <string>
#include <Windows.h>

class Window
{
public:
	// Initializes the window. May throw.
	explicit Window(
		const std::wstring& name,
		uint clientWidth, 
		uint clientHieght,
		HINSTANCE hInstance);

	~Window();
	LRESULT handleMsg(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	// Copying Window not allowed.
	Window(const Window& rhs) = delete;
	Window& operator=(const Window& rhs) = delete;

private:
	void registerWindowClass();
	void createWindow();
	void onResize();

// State
public:
	const std::wstring name;
protected:
	HINSTANCE hInstance;
	HWND windowHandle;
	uint clientWidth;
	uint clientHeight;
};
