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
	void buildRootSignature();

	void setUpRootSignatureForDraw(); // TODO figure out what to do with this. 

	void draw() override;

	DemoBox boxMesh;

	// TODO: UPLOAD BUFFER
	unique_ptr<UploadBuffer<XMFLOAT4X4>> worldViewProjBuffer;
	ComPtr<ID3D12DescriptorHeap> cbvHeap = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;

};


DemoWindow::DemoWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	D3DWindow(name, clientWidth, clientHeight, hInstance)
{
	this->boxMesh.loadGeometry(this->device.Get(), this->cmdList.Get());
	this->buildBufferForShaders();
	this->buildRootSignature();

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
DemoWindow::buildRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParam[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	rootParam[0].InitAsDescriptorTable(1, &cbvTable);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(
		1,
		rootParam,
		0,
		nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	hrThrowIfFailed(
		D3D12SerializeRootSignature(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()),
		errorBlob);

	hrThrowIfFailed(this->device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(this->rootSignature.GetAddressOf())));
}

void
DemoWindow::setUpRootSignatureForDraw()
{
	ID3D12DescriptorHeap* descriptorHeaps[] = { this->cbvHeap.Get() };
	this->cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	this->cmdList->SetGraphicsRootSignature(this->rootSignature.Get());

	this->cmdList->SetGraphicsRootDescriptorTable(0, this->cbvHeap->GetGPUDescriptorHandleForHeapStart());
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

	this->setUpRootSignatureForDraw();

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