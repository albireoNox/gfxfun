#include <lib/window.h>
#include <iostream> 

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>
#include "lib/d3dSetup.h"

using namespace std;

void run(HINSTANCE hInstance)
{
	wstring name = L"This is a window";
	Window window(name, 500, 500, hInstance);

	wcout << "Name: " << window.name << endl;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstace, LPSTR lpCmdLine, int nCmdShow)
{
	setUpDebugWindow();
	run(hInstance);
	tearDownDebugWindow();

	if (_CrtDumpMemoryLeaks())
		throw "Memory Leak Detected";

	system("pause");
	return 0;
 }