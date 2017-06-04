#pragma once

#include <comdef.h> 
#include <dxgi.h>
#include <wrl.h>

inline void throwIfHrFailed(HRESULT hr)
{
	if (FAILED(hr)) 
		throw _com_error(hr);
}

void initDxgiFactory(Microsoft::WRL::ComPtr<IDXGIFactory>);

void foo();