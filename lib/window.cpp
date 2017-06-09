#include "window.h"

#include "msg_debug.h"
#include <Windows.h>
#include <Windowsx.h>
#include <iostream>

using namespace std;

static const wchar_t* WINDOW_CLASS = L"MainWindow";

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
		wcout << "KEY PRESS: " << static_cast<wchar_t>(wParam) << endl;
		return 0;

	case WM_LBUTTONDOWN:
		wcout << "LEFT MOUSE BUTTON DOWN @ ("
			<< GET_X_LPARAM(lParam) << ", "
			<< GET_Y_LPARAM(lParam) << ")" << endl;
		return 0;

	case WM_SIZE:
		// Save the new client area dimensions.
		this->clientWidth  = LOWORD(lParam);
		this->clientHeight = HIWORD(lParam);
		this->onResize();
		wcout << "RESIZE - Window is now " << this->clientWidth << "X" << this->clientHeight << endl;
		return 0;

	case WM_DESTROY:
		wcout << "DESTROYING WINDOW" << endl;
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(this->windowHandle, msg, wParam, lParam);
	}
}

void
Window::onResize()
{
	
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
	wc.lpszClassName = WINDOW_CLASS;

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
		WINDOW_CLASS,
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
}

Window::Window(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	name(name), hInstance(hInstance), clientWidth(clientWidth), clientHeight(clientWidth)
{
	this->registerWindowClass();
	this->createWindow();
}

Window::~Window()
{
	// NOOP (for now)
}
