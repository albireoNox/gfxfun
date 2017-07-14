#include "d3d_util.h"

#include "common.h"
#include <vector>
#include <iostream>
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
using namespace std;

FILE *stdoutStream;
FILE *stderrStream;

void
setUpDebugWindow()
{
	AllocConsole();
	freopen_s(&stdoutStream, "CONOUT$", "w", stdout);
	freopen_s(&stderrStream, "CONOUT$", "w", stderr);
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTitle(L"Debug Output");

	// give the console window a bigger buffer size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(consoleHandle, &csbi))
	{
		COORD bufferSize;
		bufferSize.X = csbi.dwSize.X;
		bufferSize.Y = 9999;
		SetConsoleScreenBufferSize(consoleHandle, bufferSize);
	}
}

void
tearDownDebugWindow()
{
	fclose(stdoutStream);
	fclose(stderrStream);
}

ComPtr<IDXGIFactory4>
getDxgiFactory()
{
	ComPtr<IDXGIFactory4> out;
	hrThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(out.GetAddressOf())));
	return out;
}

vector<ComPtr<IDXGIAdapter>>
getAdapterList(IDXGIFactory* factory)
{
	vector<ComPtr<IDXGIAdapter>> out;
	uint i = 0;
	while (true)
	{
		ComPtr<IDXGIAdapter> adapter;
		if (hrThrowIfFailed(factory->EnumAdapters(i++, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND))
			break;
		out.push_back(adapter);
	}
	return out;
}

vector<ComPtr<IDXGIOutput>>
getAdapterOutputList(IDXGIAdapter* adapter)
{
	vector<ComPtr<IDXGIOutput>> out;
	uint i = 0;
	while (true)
	{
		ComPtr<IDXGIOutput> output;
		if (hrThrowIfFailed(adapter->EnumOutputs(i++, output.GetAddressOf()) == DXGI_ERROR_NOT_FOUND))
			break;
		out.push_back(output);
	}
	return out;
}

vector<DXGI_MODE_DESC>
getOutputDisplayModeList(IDXGIOutput* output)
{
	UINT count = 0;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT flags = DXGI_ENUM_MODES_INTERLACED;
	vector<DXGI_MODE_DESC> out;

	hrThrowIfFailed(output->GetDisplayModeList(format, flags, &count, nullptr));
	out.resize(count);
	if (count > 0)
		hrThrowIfFailed(output->GetDisplayModeList(format, flags, &count, &out[0]));
	return out;
}

ComPtr<ID3D12Device>
getDevice(IDXGIAdapter* adapter)
{
	ComPtr<ID3D12Device> device;
	hrThrowIfFailed(D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(device.GetAddressOf())));
	return device;
}

ComPtr<ID3D12Device>
getDefaultDevice(IDXGIFactory4* factory)
{
	ComPtr<ID3D12Device> device;
	HRESULT hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(device.GetAddressOf()));

	// Fallback to WARP device.
	if (FAILED(hr))
	{
		wcout << "Failed to create device: falling back to WARP device." << endl;

		ComPtr<IDXGIAdapter> warpAdapter;
		hrThrowIfFailed(factory->EnumWarpAdapter(
			IID_PPV_ARGS(warpAdapter.GetAddressOf())));

		hrThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(device.GetAddressOf())));
	}
	return device;
}

ComPtr<ID3D12Fence>
getFence(ID3D12Device* device)
{
	ComPtr<ID3D12Fence> fence;
	hrThrowIfFailed(device->CreateFence(
		0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf())));
	return fence;
}

ComPtr<ID3D12CommandQueue>
getCommandQueue(ID3D12Device* device)
{
	D3D12_COMMAND_QUEUE_DESC desc = { };
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ComPtr<ID3D12CommandQueue> commandQueue;
	hrThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(commandQueue.GetAddressOf())));
	return commandQueue;
}

ComPtr<ID3D12CommandAllocator>
getCommandAllocator(ID3D12Device* device)
{
	ComPtr<ID3D12CommandAllocator> cmdAllocator;
	hrThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdAllocator.GetAddressOf())));
	return cmdAllocator;
}

ComPtr<ID3D12GraphicsCommandList>
getCommandList(ID3D12Device* device, ID3D12CommandAllocator* cmdAllocator)
{
	ComPtr<ID3D12GraphicsCommandList> cmdList;
	hrThrowIfFailed(device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		cmdAllocator,
		nullptr,
		IID_PPV_ARGS(cmdList.GetAddressOf())));

	cmdList->Close();
	return cmdList;
}

uint
getMSAAQualityLevels(ID3D12Device* device, uint sampleCount)
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
	qualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	qualityLevels.SampleCount = sampleCount;
	qualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	qualityLevels.NumQualityLevels = 0;
	hrThrowIfFailed(device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(qualityLevels)));
	return qualityLevels.NumQualityLevels;
}

ComPtr<ID3D12Resource> 
createDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 bufferByteCount,
	ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	// Create the actual default buffer resource.
	hrThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferByteCount),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));
	SetDebugObjectName(defaultBuffer.Get(), L"DEFAULT BUFFER");

	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	hrThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferByteCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));
	SetDebugObjectName(uploadBuffer.Get(), L"UPLOAD BUFFER");


	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = bufferByteCount;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// Note: uploadBuffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy has been executed.

	return defaultBuffer;
}