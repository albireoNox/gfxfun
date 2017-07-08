#include "lib/d3d_window.h"
#include "lib/d3d_util.h"
#include <DirectXMath.h>
#include <Windows.h>

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;
using namespace DirectX;

/// DEMO WINDOW ///

class DemoWindow : public D3DWindow
{
public:
	DemoWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance);
	void update();

protected:
	void draw() override;
};


DemoWindow::DemoWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	D3DWindow(name, clientWidth, clientHeight, hInstance)
{
	// NOOP
}

void
DemoWindow::update()
{
	this->render();
}

void
DemoWindow::draw()
{

}

void
runMsgLoop(DemoWindow& window)
{
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			window.update();
		}
	}
}

void
run(HINSTANCE hInstance)
{
	wstring name = L"Simple Drawing Demo";
	DemoWindow window(name, 800, 800, hInstance);
	window.show();

	runMsgLoop(window);
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