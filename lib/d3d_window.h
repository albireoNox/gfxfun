#pragma once

#include "window.h"
#include <d3d12.h>
#include <DXGI1_4.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <d3d11On12.h>
#include <d2d1_1.h>
#include <d2d1_3.h>

class D3DWindow : public Window
{
public:
	static const uint SWAPCHAIN_BUFFER_COUNT = 2;
	// TODO parameterize these.
	static const uint MSAA_SAMPLE_COUNT = 1;
	static const uint MSAA_QUALITY_LEVEL = 0;

	DXGI_FORMAT DEPTH_STENCIL_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;

	// Initializes the window. May throw.
	D3DWindow(
		const std::wstring& name,
		uint clientWidth,
		uint clientHeight,
		HINSTANCE hInstance);

	~D3DWindow();

	virtual void draw();

protected:
	D3D12_CPU_DESCRIPTOR_HANDLE getCurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE getDepthStencilView() const;

	void onResize(uint newClientWidth, uint newClientHieght) override;
	void presentAndAdvanceSwapchain();

private:
	void clearRenderTargets();
	void initializeRenderTargets();
	void initializeDepthStencilBuffer();
	void createSwapChain();
	void createDescriptorHeaps();
	void initD2d();

	void flush();

	// State
private:
	Microsoft::WRL::ComPtr<IDXGIFactory4>             factory;
	Microsoft::WRL::ComPtr<ID3D12Device>              device;

	Microsoft::WRL::ComPtr<ID3D12Fence>               fence;
	uint                                              currentFence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue>        cmdQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    cmdAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
	Microsoft::WRL::ComPtr<IDXGISwapChain>            swapChain;

	// Render Target View
	uint                                         rtvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>       swapChainBuffer[SWAPCHAIN_BUFFER_COUNT];
	uint currentBackBuffer;

	// Depth/Stencil View
	uint                                         dsvDescriptorSize;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource>       depthStencilBuffer;

	uint cbvSrvDescriptorSize;

	// D2D Stuff
	Microsoft::WRL::ComPtr<ID3D11Resource>      wrappedSwapChainBuffer[SWAPCHAIN_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<IDXGISurface>        d2dSurfaces[SWAPCHAIN_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID2D1Bitmap1>        d2dRenderTargets[SWAPCHAIN_BUFFER_COUNT];
	Microsoft::WRL::ComPtr<ID3D11Device>        d3d11Device;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext>  d2dDeviceContext;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dDeviceContext;
	Microsoft::WRL::ComPtr<ID3D11On12Device>    d3d11On12Device;
	Microsoft::WRL::ComPtr<ID2D1Device2>        d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1Factory3>       d2dFactory;

	float dpiX;
	float dpiY;
};
