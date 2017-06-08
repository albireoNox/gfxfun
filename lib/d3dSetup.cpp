#include "d3dSetup.h"

#include "common.h"
#include <vector>
#include <iostream>

using Microsoft::WRL::ComPtr;
using namespace std;

FILE *stdoutStream;
FILE *stderrStream;

void setUpDebugWindow()
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

void tearDownDebugWindow()
{
	fclose(stdoutStream);
	fclose(stderrStream);
}

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
