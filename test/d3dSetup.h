#pragma once

#include <comdef.h> 
#include <dxgi.h>
#include <wrl.h>
#include <vector>

inline HRESULT hrThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr)) 
		throw _com_error(hr);
	return hr;
}

void initDxgiFactory(Microsoft::WRL::ComPtr<IDXGIFactory>&);
void populateAdapterList(IDXGIFactory*, std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>>& out);
void populateAdapterOutputList(IDXGIAdapter*, std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>>& out);
void populateOutputDisplayModeList(IDXGIOutput*, std::vector<DXGI_MODE_DESC>& out);