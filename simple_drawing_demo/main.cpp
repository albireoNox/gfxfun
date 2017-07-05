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

struct Vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;

	static const std::vector<D3D12_INPUT_ELEMENT_DESC> LAYOUT_FIELDS;
	static const D3D12_INPUT_LAYOUT_DESC LAYOUT_DESC;
};

const std::vector<D3D12_INPUT_ELEMENT_DESC> Vertex::LAYOUT_FIELDS(
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(XMFLOAT3),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	});

const D3D12_INPUT_LAYOUT_DESC Vertex::LAYOUT_DESC =
	{ Vertex::LAYOUT_FIELDS.data(), Vertex::LAYOUT_FIELDS.size() };

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