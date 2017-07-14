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

// Shaders
#include "VertexShader.h"
#include "PixelShader.h"
#include "lib/math.h"

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
	void buildPSO();

	void setUpRootSignatureForDraw(); // TODO figure out what to do with this. 

	ID3D12PipelineState* getPso() override;
	void onResize(uint newX, uint newY) override;
	void onCharInput(wchar_t ch) override;
	void draw() override;

	DemoBox boxMesh;

	// TODO: UPLOAD BUFFER
	unique_ptr<UploadBuffer<XMFLOAT4X4>> worldViewProjBuffer;
	ComPtr<ID3D12DescriptorHeap> cbvHeap = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	ComPtr<ID3D12PipelineState> pso = nullptr;

	// VIEW DATA
	XMFLOAT4X4 mxWorld = Math::Identity4x4();
	XMFLOAT4X4 mxView = Math::Identity4x4();
	XMFLOAT4X4 mxProj = Math::Identity4x4();

	float mTheta = 1.5f*XM_PI;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;
};


DemoWindow::DemoWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	D3DWindow(name, clientWidth, clientHeight, hInstance)
{
	this->boxMesh.loadGeometry(this->device.Get(), this->cmdList.Get());
	this->buildBufferForShaders();
	this->buildRootSignature();
	this->buildPSO();
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
DemoWindow::buildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = Vertex::LAYOUT_DESC;
	psoDesc.pRootSignature = this->rootSignature.Get();
	psoDesc.VS = { VS_BYTE_CODE, sizeof(VS_BYTE_CODE) };
	psoDesc.PS = { PS_BYTE_CODE, sizeof(PS_BYTE_CODE) };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = D3DWindow::BACK_BUFFER_FORMAT;
	psoDesc.SampleDesc.Count = D3DWindow::MSAA_SAMPLE_COUNT;
	psoDesc.SampleDesc.Quality = D3DWindow::MSAA_QUALITY_LEVEL;
	psoDesc.DSVFormat = D3DWindow::DEPTH_STENCIL_FORMAT;
	hrThrowIfFailed(this->device->CreateGraphicsPipelineState(
		&psoDesc, IID_PPV_ARGS(this->pso.GetAddressOf())));
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
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&this->mxView, view);

	XMMATRIX world = XMLoadFloat4x4(&this->mxWorld);
	XMMATRIX proj = XMLoadFloat4x4(&this->mxProj);
	XMMATRIX worldViewProj = world*view*proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	XMFLOAT4X4 mxOut;
	XMStoreFloat4x4(&mxOut, XMMatrixTranspose(worldViewProj));

	this->worldViewProjBuffer->copyData(0, mxOut);

	this->render();
}

void 
DemoWindow::onResize(uint newX, uint newY)
{
	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*Math::Pi, (float)newX / newY, 1.0f, 1000.0f);
	XMStoreFloat4x4(&this->mxProj, P);
	__super::onResize(newX, newY);
}

ID3D12PipelineState*
DemoWindow::getPso()
{
	return this->pso.Get();
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

void DemoWindow::onCharInput(wchar_t ch)
{
	const float delta = 0.05;
	switch (ch)
	{
	case 'a':
		this->mTheta += delta;
		break;
	case 'd':
		this->mTheta -= delta;
		break;
	case 'w':
		this->mPhi += delta;
		break;
	case 's':
		this->mPhi -= delta;
		break;
	}
	__super::onCharInput(ch);
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