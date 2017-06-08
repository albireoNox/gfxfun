#include <lib/window.h>
#include <iostream> 

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;

void run(HINSTANCE hInstance)
{
	wstring name = L"This is a window";
	Window window(name, 500, 500, hInstance);

	wcout << "Name: " << window.name << endl;
}

FILE *stdoutStream;
FILE *stderrStream;

void setUpDebugWindow()
{
	AllocConsole();
	freopen_s(&stdoutStream, "CONOUT$", "w", stdout);
	freopen_s(&stderrStream, "CONOUT$", "w", stderr);
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTitle(L"Debug Output");

	// give the console window a bigger buffer size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(consoleHandle, &csbi))
	{
		COORD bufferSize;
		bufferSize.X = csbi.dwSize.X;
		bufferSize.Y = 9999;
		SetConsoleScreenBufferSize(consoleHandle, bufferSize);
	}
}

void tearDownDebugWindow()
{
	fclose(stdoutStream);
	fclose(stderrStream);
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