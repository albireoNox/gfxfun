#pragma once

#include <comdef.h> 
#include <dxgi.h>
#include <wrl.h>
#include <vector>

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

Microsoft::WRL::ComPtr<IDXGIFactory>
getDxgiFactory();

std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter>>
getAdapterList(IDXGIFactory*);

std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>>
getAdapterOutputList(IDXGIAdapter*);

std::vector<DXGI_MODE_DESC>
getOutputDisplayModeList(IDXGIOutput*);