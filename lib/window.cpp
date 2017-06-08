#include "window.h"

#include <Windows.h>
#include <iostream>

using namespace std;

static const wchar_t* WINDOW_CLASS = L"MainWindow";

LRESULT CALLBACK
handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Window::registerWindowClass()
{
	WNDCLASS wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = handleMsg;
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

void Window::createWindow()
{
	RECT R = { 0, 0, this->clientWidth, this->clientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	cout << "Creating window with final size: " << width << "X" << height << endl;

	this->windowHandle = CreateWindow(
		WINDOW_CLASS,
		this->name.c_str()	,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
