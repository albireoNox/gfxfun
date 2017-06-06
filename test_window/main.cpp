#include <lib/window.h>
#include <iostream> 

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;

void run()
{
	wstring name = L"This is a window";
	Window window(name);

	wcout << "Name: " << window.name << endl;
}

int main()
 {
	run();

	if (_CrtDumpMemoryLeaks())
		throw "Memory Leak Detected";

	system("pause");
	return 0;
 }