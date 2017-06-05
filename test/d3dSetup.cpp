#include "d3dSetup.h"

#include "common.h"
#include <vector>
#include <iostream>

using Microsoft::WRL::ComPtr;

void initDxgiFactory(ComPtr<IDXGIFactory> & out)
{
	hrThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)out.GetAddressOf()));
}

void populateAdapterList(IDXGIFactory * factory, std::vector<ComPtr<IDXGIAdapter>> * out)
{
	uint i = 0;
	while (true)
	{
		ComPtr<IDXGIAdapter> adapter;
		if (hrThrowIfFailed(factory->EnumAdapters(i++, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND))
			break;
		out->push_back(adapter);
	}
}

void foo()
{
	std::cout << "This is a test." << std::endl;
}