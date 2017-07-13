#include "lib/d3d_window.h"
#include "lib/d3d_util.h"
#include "box.h"
#include <DirectXMath.h>
#include <Windows.h>

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>
#include <DirectXColors.h>
#include <lib/upload_buffer.h>
#include <memory>

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

/// DEMO WINDOW ///

class DemoWindow : public D3DWindow
{
public:
	DemoWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance);
	void update();

protected:
	// TODO: UPLOAD BUFFER
	void buildBufferForShaders();

	void draw() override;

	DemoBox boxMesh;

	// TODO: UPLOAD BUFFER
	unique_ptr<UploadBuffer<XMFLOAT4X4>> worldViewProjBuffer;
	ComPtr<ID3D12DescriptorHeap> cbvHeap = nullptr;

};


DemoWindow::DemoWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	D3DWindow(name, clientWidth, clientHeight, hInstance)
{
	this->boxMesh.loadGeometry(this->device.Get(), this->cmdList.Get());
	this->buildBufferForShaders();

	this->flush();
	this->boxMesh.cleanUpLoadArtifacts();
}

// TODO: UPLOAD BUFFER
void
DemoWindow::buildBufferForShaders()
{
	this->worldViewProjBuffer =
		std::make_unique<UploadBuffer<XMFLOAT4X4>>(this->device.Get(), 1, true);
	
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	hrThrowIfFailed(this->device->CreateDescriptorHeap(
		&cbvHeapDesc,
		IID_PPV_ARGS(this->cbvHeap.GetAddressOf())));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = worldViewProjBuffer->resource()->GetGPUVirtualAddress();
	const int bufferIndex = 0; // TODO: If we make the buffer a utility, will need to change this. 
	cbAddress += bufferIndex * this->worldViewProjBuffer->byteCountPerElement;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = this->worldViewProjBuffer->byteCountPerElement;

	this->device->CreateConstantBufferView(
		&cbvDesc,
		this->cbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void
DemoWindow::update()
{
	this->render();
}

void
DemoWindow::draw()
{
	this->cmdList->ClearRenderTargetView(
		this->getCurrentBackBufferView(),
		DirectX::Colors::DarkSlateGray,
		0,
		nullptr);

	this->boxMesh.draw(this->cmdList.Get());
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