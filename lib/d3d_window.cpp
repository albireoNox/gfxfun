#include "d3d_window.h"

#include "d3d_util.h"
#include "d3dx12.h"
#include <assert.h>
#include <d3d12.h>
#include <d3d11On12.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <sstream>
#include <wrl/client.h>
#include <DirectXColors.h>
#include <iomanip>

using Microsoft::WRL::ComPtr;
using namespace std;

D3DRenderTarget::D3DRenderTarget(
	ID3D12Device* device, 
	ID3D11On12Device* d3d11On12Device,
	ID2D1Device2 *d2dDevice,
	IDXGISwapChain* swapchain, 
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle, 
	uint bufferIndex,
	float dpiX,
	float dpiY)
{
	// Get resource 
	hrThrowIfFailed(swapchain->GetBuffer(
		bufferIndex, IID_PPV_ARGS(this->swapChainBuffer.GetAddressOf())));

	// Create descriptor
	device->CreateRenderTargetView(
		this->swapChainBuffer.Get(), nullptr, rtvHeapHandle);

	// D2D
	D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };

	hrThrowIfFailed(d3d11On12Device->CreateWrappedResource(
		this->swapChainBuffer.Get(),
		&d3d11Flags,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT,
		IID_PPV_ARGS(this->wrappedSwapChainBuffer.GetAddressOf())));

	hrThrowIfFailed(this->wrappedSwapChainBuffer.As(&this->d2dSurface));

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY);

	hrThrowIfFailed(
		d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			d2dDeviceContext.GetAddressOf()));

	hrThrowIfFailed(this->d2dDeviceContext->CreateBitmapFromDxgiSurface(
		this->d2dSurface.Get(),
		&bitmapProperties,
		this->d2dRenderTarget.GetAddressOf()));
}

D3DRenderTarget::~D3DRenderTarget()
{
	// Use default destructors.
}

void enableDebugLayer()
{
#if defined(DEGUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	hrThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
#endif
}

D3DWindow::D3DWindow(const wstring& name, uint clientWidth, uint clientHeight, HINSTANCE hInstance) :
	Window(name, clientWidth, clientHeight, hInstance)
{
	enableDebugLayer();

	this->factory = getDxgiFactory();
	this->device = getDefaultDevice(this->factory.Get());
	this->fence = getFence(device.Get());

	if (getMSAAQualityLevels(this->device.Get(), 4) == 0)
		throw "4x MSAA Not Supported";

	this->cmdQueue = getCommandQueue(this->device.Get());
	this->cmdAllocator = getCommandAllocator(this->device.Get());
	this->cmdList = getCommandList(this->device.Get(), this->cmdAllocator.Get());
	this->createSwapChain();
	this->currentBackBuffer = 0;

	this->rtvDescriptorSize =
		this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	this->dsvDescriptorSize =
		this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	this->cbvSrvDescriptorSize =
		this->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	this->createDescriptorHeaps();
	this->initD2d();

	this->fpsStatStopwatch.start();
}

D3DWindow::~D3DWindow()
{
	// NOOP (for now)
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

D3DRenderTarget&
D3DWindow::currentRenderTarget()
{
	return this->renderTargets[this->currentBackBuffer];
}

void
D3DWindow::onCreate()
{
	__super::onCreate();
	this->onResize(this->clientWidth, this->clientHeight);
}

void
D3DWindow::onResize(uint newClientWidth, uint newClientHeight)
{
	__super::onResize(newClientWidth, newClientHeight);

	assert(this->device);
	assert(this->swapChain);
	assert(this->cmdAllocator);

	this->clearRenderTargets();

	// Flush before changing any resources.
	this->flush();

	hrThrowIfFailed(this->cmdList->Reset(this->cmdAllocator.Get(), nullptr));

	this->initializeRenderTargets();
	this->initializeDepthStencilBuffer();

	hrThrowIfFailed(this->cmdList->Close());
	ID3D12CommandList* cmdsLists[] = { this->cmdList.Get() };
	this->cmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	this->render();

	// Wait until resize is complete.
	this->flush();
}

void
D3DWindow::updateStats()
{
	this->totalFramesRendered++;
	this->framesRenderedThisSecond++;

	double fpsUpdateWindowSeconds = this->fpsStatStopwatch.elapsedSeconds();
	if (fpsUpdateWindowSeconds >= 1.0)
	{
		this->fps = this->framesRenderedThisSecond / fpsUpdateWindowSeconds;
		this->fpsStatStopwatch.reset();
		this->framesRenderedThisSecond = 0;
	}
	wstringstream statsString;
	statsString 
		<< this->name << L": "
		<< L"fps: "   << setprecision(1) << fixed << this->fps << L" "
		<< L"frame: " << this->totalFramesRendered;

	SetWindowText(this->windowHandle, statsString.str().c_str());
}

void
D3DWindow::render()
{
	// Reset command list objects. 
	hrThrowIfFailed(this->cmdAllocator->Reset());
	hrThrowIfFailed(this->cmdList->Reset(this->cmdAllocator.Get(), this->getPso()));

	this->cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			this->currentRenderTarget().swapChainBuffer.Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET));

	this->cmdList->ClearDepthStencilView(
		this->getDepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		0,
		nullptr);

	this->cmdList->OMSetRenderTargets(
		1,
		&this->getCurrentBackBufferView(),
		true,
		&this->getDepthStencilView());

	// Defer to subclass for specific drawing 
	this->draw();

	this->cmdList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			this->currentRenderTarget().swapChainBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	// Execute command list
	hrThrowIfFailed(this->cmdList->Close());
	ID3D12CommandList* cmdsLists[] = { this->cmdList.Get() };
	this->cmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	this->d3dDeviceContext->Flush();
	this->flush();

	hrThrowIfFailed(this->swapChain->Present(1, 0));
	this->currentBackBuffer =
		(this->currentBackBuffer + 1) % D3DWindow::SWAPCHAIN_BUFFER_COUNT;

	this->updateStats();
}

void
D3DWindow::clearRenderTargets()
{
	// Release the previous resources we will be recreating.
	this->renderTargets.clear();
	this->d3dDeviceContext->Flush();
}

void
D3DWindow::initializeRenderTargets()
{
	// Resize the swap chain.
	hrThrowIfFailed(this->swapChain->ResizeBuffers(
		D3DWindow::SWAPCHAIN_BUFFER_COUNT,
		this->clientWidth,
		this->clientHeight,
		D3DWindow::BACK_BUFFER_FORMAT,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	this->currentBackBuffer = 0;

	// Create Render Target View
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(
		this->rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (uint i = 0; i < D3DWindow::SWAPCHAIN_BUFFER_COUNT; i++)
	{
		this->renderTargets.push_back(D3DRenderTarget(
			this->device.Get(),
			this->d3d11On12Device.Get(),
			this->d2dDevice.Get(),
			this->swapChain.Get(),
			rtvHeapHandle,
			i,
			this->dpiX,
			this->dpiY));

		rtvHeapHandle.Offset(1, this->rtvDescriptorSize);
	}
}

void
D3DWindow::initializeDepthStencilBuffer()
{
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

void
D3DWindow::createDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = D3DWindow::SWAPCHAIN_BUFFER_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	hrThrowIfFailed(this->device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(this->rtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	hrThrowIfFailed(this->device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(this->dsvHeap.GetAddressOf())));
}

void
D3DWindow::initD2d()
{
	hrThrowIfFailed(D3D11On12CreateDevice(
		this->device.Get(),
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(this->cmdQueue.GetAddressOf()),
		1,
		0,
		&this->d3d11Device,
		&this->d3dDeviceContext,
		nullptr));

	hrThrowIfFailed(this->d3d11Device.As(&this->d3d11On12Device));

	hrThrowIfFailed(
		D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory3),
			nullptr,
			&this->d2dFactory));
	this->d2dFactory->GetDesktopDpi(&this->dpiX, &this->dpiY);

	ComPtr<IDXGIDevice> dxgiDevice;
	hrThrowIfFailed(this->d3d11On12Device.As(&dxgiDevice));

	hrThrowIfFailed(
		this->d2dFactory->CreateDevice(dxgiDevice.Get(), &this->d2dDevice));
}

void
D3DWindow::flush()
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

ID3D12PipelineState*
D3DWindow::getPso()
{
	return nullptr;
}