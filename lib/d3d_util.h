#pragma once

#include <comdef.h> 
#include <dxgi.h>
#include <wrl.h>
#include <vector>
#include <d3d12.h>
#include <DXGI1_4.h>

inline HRESULT
hrThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) 
		throw _com_error(hr);
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