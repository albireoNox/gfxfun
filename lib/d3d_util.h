#pragma once

#include "common.h"
#include <comdef.h> 
#include <dxgi.h>
#include <wrl.h>
#include <vector>
#include <d3d12.h>
#include <DXGI1_4.h>
#include <iostream>

inline HRESULT
hrThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) {
		auto err = _com_error(hr);
		std::wcout << err.ErrorMessage() << std::endl;
		throw err;
	}

	return hr;
}

void
setUpDebugWindow();

void
tearDownDebugWindow();

Microsoft::WRL::ComPtr<IDXGIFactory4>
getDxgiFactory();

std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>>
getAdapterList(IDXGIFactory*);

std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>>
getAdapterOutputList(IDXGIAdapter*);

std::vector<DXGI_MODE_DESC>
getOutputDisplayModeList(IDXGIOutput*);

Microsoft::WRL::ComPtr<ID3D12Device>
getDevice(IDXGIAdapter*);

Microsoft::WRL::ComPtr<ID3D12Device>
getDefaultDevice(IDXGIFactory4*);

Microsoft::WRL::ComPtr<ID3D12Fence>
getFence(ID3D12Device*);

Microsoft::WRL::ComPtr<ID3D12CommandQueue>
getCommandQueue(ID3D12Device*);

Microsoft::WRL::ComPtr<ID3D12CommandAllocator>
getCommandAllocator(ID3D12Device*);

Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>
getCommandList(ID3D12Device*, ID3D12CommandAllocator*);

uint
getMSAAQualityLevels(ID3D12Device*, uint sampleCount);

Microsoft::WRL::ComPtr<ID3D12Resource>
createDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 bufferByteCount,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);