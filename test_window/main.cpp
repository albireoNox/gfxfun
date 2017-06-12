#include "lib/window.h"
#include "lib/d3d_window.h"
#include "lib/d3d_util.h"
#include <iostream> 

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;

void
runMsgLoop()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void
run(HINSTANCE hInstance)
{
	wstring name = L"This is a window";
	D3DWindow window(name, 500, 500, hInstance);

	wstring otherName = L"Other window";
	Window otherWindow(otherName, 600, 100, hInstance);

	runMsgLoop();
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	setUpDebugWindow();
	run(hInstance);
	tearDownDebugWindow();

	system("pause");
	return 0;
 }