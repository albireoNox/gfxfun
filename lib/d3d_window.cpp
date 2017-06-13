#include "d3d_window.h"

#include "d3d_util.h"
#include "d3dx12.h"
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

void
D3DWindow::flushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	this->currentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	hrThrowIfFailed(this->cmdQueue->Signal(this->fence.Get(), this->currentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (this->fence->GetCompletedValue() < this->currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		hrThrowIfFailed(
			this->fence->SetEventOnCompletion(this->currentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void
D3DWindow::onResize(uint newClientWidth, uint newClientHeight)
{
	__super::onResize(newClientWidth, newClientHeight);

//	assert(md3dDevice);
//	assert(mSwapChain);
//	assert(mDirectCmdListAlloc);

	// Flush before changing any resources.
	this->flushCommandQueue();

	hrThrowIfFailed(this->cmdList->Reset(this->cmdAllocator.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < D3DWindow::SWAPCHAIN_BUFFER_COUNT; ++i)
		this->swapChainBuffer[i].Reset();
	this->depthStencilBuffer.Reset();

	// Resize the swap chain.
	hrThrowIfFailed(this->swapChain->ResizeBuffers(
		D3DWindow::SWAPCHAIN_BUFFER_COUNT,
		newClientWidth,
		newClientHeight,
		D3DWindow::BACK_BUFFER_FORMAT,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	this->currentBackBuffer = 0;

	// Create Render Target View
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(
		this->rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (uint i = 0; i < D3DWindow::SWAPCHAIN_BUFFER_COUNT; i++)
	{
		// Get resource 
		hrThrowIfFailed(this->swapChain->GetBuffer(
			i, IID_PPV_ARGS(this->swapChainBuffer[i].GetAddressOf())));

		// Create descriptor
		this->device->CreateRenderTargetView(
			this->swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);

		rtvHeapHandle.Offset(1, this->rtvDescriptorSize);
	}

	// Create Depth/Stencil Buffer and View
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = this->clientWidth;
	depthStencilDesc.Height = this->clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = D3DWindow::DEPTH_STENCIL_FORMAT;
	depthStencilDesc.SampleDesc.Count = D3DWindow::MSAA_SAMPLE_COUNT;
	depthStencilDesc.SampleDesc.Quality = D3DWindow::MSAA_QUALITY_LEVEL;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = D3DWindow::DEPTH_STENCIL_FORMAT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	hrThrowIfFailed(this->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(this->depthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	this->device->CreateDepthStencilView(
		this->depthStencilBuffer.Get(), nullptr, this->getDepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	this->cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			this->depthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE));

	// Execute the resize commands.
	hrThrowIfFailed(this->cmdList->Close());
	ID3D12CommandList* cmdsLists[] = { this->cmdList.Get() };
	this->cmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	this->flushCommandQueue();

	hrThrowIfFailed(this->swapChain->Present(0, 0));
}

D3D12_CPU_DESCRIPTOR_HANDLE
D3DWindow::getCurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		this->rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		this->currentBackBuffer, 
		this->rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE
D3DWindow::getDepthStencilView() const
{
	return this->dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void enableDebugLayer()
{
#if defined(DEGUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	hrThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
#endif
}

void
D3DWindow::createSwapChain()
{
	// Release the previous swapchain we will be recreating.
	this->swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = this->clientWidth;
	sd.BufferDesc.Height = this->clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = D3DWindow::BACK_BUFFER_FORMAT;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = D3DWindow::MSAA_SAMPLE_COUNT;
	sd.SampleDesc.Quality = D3DWindow::MSAA_QUALITY_LEVEL;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = D3DWindow::SWAPCHAIN_BUFFER_COUNT;
	sd.OutputWindow = this->windowHandle;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	hrThrowIfFailed(this->factory->CreateSwapChain(
		this->cmdQueue.Get(),
		&sd,
		this->swapChain.GetAddressOf()));
}

void D3DWindow::createDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = D3DWindow::SWAPCHAIN_BUFFER_COUNT;
	rtvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask       = 0;
	hrThrowIfFailed(this->device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(this->rtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask       = 0;
	hrThrowIfFailed(this->device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(this->dsvHeap.GetAddressOf())));
}

D3DWindow::D3DWindow(const std::wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	Window(name, clientWidth, clientHeight, hInstance)
{
	this->factory = getDxgiFactory();
	this->device  = getDefaultDevice(this->factory.Get());
	this->fence   = getFence(device.Get());

	if (getMSAAQualityLevels(this->device.Get(), 4) == 0)
		throw "4x MSAA Not Supported";

	this->cmdQueue     = getCommandQueue(this->device.Get());
	this->cmdAllocator = getCommandAllocator(this->device.Get());
	this->cmdList      = getCommandList(this->device.Get(), this->cmdAllocator.Get());
	this->createSwapChain();
	this->currentBackBuffer = 0;

	this->rtvDescriptorSize =
		this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	this->dsvDescriptorSize =
		this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	this->cbvSrvDescriptorSize =
		this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	this->createDescriptorHeaps();

	this->onResize(clientWidth, clientHeight);
}

D3DWindow::~D3DWindow()
{
	// NOOP (for now)
}
