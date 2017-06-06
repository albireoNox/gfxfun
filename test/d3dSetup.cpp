#include "d3dSetup.h"

#include "common.h"
#include <vector>
#include <iostream>

using Microsoft::WRL::ComPtr;
using namespace std;

void initDxgiFactory(ComPtr<IDXGIFactory>& out)
{
	hrThrowIfFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)out.GetAddressOf()));
}

void populateAdapterList(IDXGIFactory* factory, vector<ComPtr<IDXGIAdapter>>& out)
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

void populateAdapterOutputList(IDXGIAdapter* adapter, vector<ComPtr<IDXGIOutput>>& out)
{
	uint i = 0;
	while (true)
	{
		ComPtr<IDXGIOutput> output;
		if (hrThrowIfFailed(adapter->EnumOutputs(i++, output.GetAddressOf()) == DXGI_ERROR_NOT_FOUND))
			break;
		out.push_back(output);
	}
}

void populateOutputDisplayModeList(IDXGIOutput* output, vector<DXGI_MODE_DESC>& out)
{
	UINT count = 0;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT flags = DXGI_ENUM_MODES_INTERLACED;

	hrThrowIfFailed(output->GetDisplayModeList(format, flags, &count, nullptr));
	out.resize(count);
	if (count == 0)
		return;
	hrThrowIfFailed(output->GetDisplayModeList(format, flags, &count, &out[0]));
}
