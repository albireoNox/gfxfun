#include "d3dSetup.h"

#include "common.h"
#include <vector>
#include <iostream>

using Microsoft::WRL::ComPtr;

void initDxgiFactory(ComPtr<IDXGIFactory> & out)
{
	hrThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)out.GetAddressOf()));
}

void populateAdapterList(IDXGIFactory * factory, std::vector<ComPtr<IDXGIAdapter>> & out)
{
	uint i = 0;
	while (true)
	{
		ComPtr<IDXGIAdapter> adapter;
		if (hrThrowIfFailed(factory->EnumAdapters(i++, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND))
			break;
		out.push_back(adapter);
	}
}

void populateAdapterOutputList(IDXGIAdapter *adapter, std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>> & out)
{
	uint i = 0;
	while (true)
	{
		ComPtr<IDXGIOutput> output;
		if (hrThrowIfFailed(adapter->EnumOutputs(i, output.GetAddressOf()) == DXGI_ERROR_NOT_FOUND))
			break;
		out.push_back(output);
	}
}