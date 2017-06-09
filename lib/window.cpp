#include "window.h"

#include "msg_debug.h"
#include <Windows.h>
#include <Windowsx.h>
#include <iostream>
#include <cassert>

using namespace std;

uint Window::numOpenWindows = 0;

LRESULT CALLBACK 
handleMsgCb(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (window != nullptr)
		return window->handleMsg(msg, wParam, lParam);
	else 
		return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT
Window::handleMsg(UINT msg, WPARAM wParam, LPARAM lParam)
{
	//wcout << "msg:" << getMsgDebugString(msg) << " w:" << wParam << " l:" << lParam << endl;
	switch(msg)
	{
	case WM_CHAR:
		this->onCharInput(static_cast<wchar_t>(wParam));
		return 0;

	case WM_LBUTTONDOWN:
		// TODO This is not really a click.
		this->onMouseLClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_SIZE:

		this->onResize(LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_DESTROY:
		this->onClose();
		return 0;

	default:
		return DefWindowProc(this->windowHandle, msg, wParam, lParam);
	}
}

void
Window::onCharInput(wchar_t ch)
{
	wcout << this->name << ": KEY PRESS " << ch << endl;
}

void
Window::onMouseLClick(int x, int y)
{
	wcout << this->name << ": LEFT MOUSE BUTTON CLICK @ (" << x << ", " << y << ")" << endl;
}

void
Window::onResize(uint newClientWidth, uint newClientHeight)
{
	// Save the new client area dimensions.
	this->clientWidth = newClientWidth;
	this->clientHeight = newClientHeight;
	wcout << this->name << ": RESIZE - Window is now "
		<< this->clientWidth << "X" << this->clientHeight << endl;
}

void
Window::onClose()
{
	wcout << this->name << ": DESTROYING WINDOW" << endl;
	assert(Window::numOpenWindows > 0);
	if (--Window::numOpenWindows == 0)
		PostQuitMessage(0);
}

void
Window::registerWindowClass()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = handleMsgCb;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = this->hInstance;
	wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = this->name.c_str();

	if (!RegisterClass(&wc))
		throw L"RegisterClass Failed.";

	cout << "Registered window class." << endl;
}

void
Window::createWindow()
{
	RECT R = { 0, 0, this->clientWidth, this->clientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	cout << "Creating window with final size: " << width << "X" << height << endl;

	this->windowHandle = CreateWindow(
		this->name.c_str(), // Use name for class name.
		this->name.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		this->hInstance,
		nullptr);

	if (this->windowHandle == nullptr)
		throw "Failed to create window";

	SetLastError(0);
	SetWindowLongPtr(this->windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	if (GetLastError() != 0)
		throw "Failed to bind window object.";

	ShowWindow(this->windowHandle, SW_SHOW);
	UpdateWindow(this->windowHandle);

	Window::numOpenWindows++;
}

Window::Window(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	name(name), hInstance(hInstance), clientWidth(clientWidth), clientHeight(clientHeight)
{
	this->registerWindowClass();
	this->createWindow();
}

Window::~Window()
{
	// NOOP (for now)
}
