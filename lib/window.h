#pragma once

#include "common.h"
#include <string>
#include <Windows.h>

class Window
{
public:
	// Initializes the window. May throw.
	Window(
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

	void onResize(uint newClientWidth, uint newClientHeight);
	void onCharInput(wchar_t);
	void onMouseLClick(int x, int y);
	void onClose();

private:
	void registerWindowClass();
	void createWindow();

// State
public:
	const std::wstring name;
protected:
	static uint numOpenWindows;

	HINSTANCE hInstance;
	HWND windowHandle;
	uint clientWidth;
	uint clientHeight;
};
