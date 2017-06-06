#include "common.h"
#include "lib/d3dSetup.h"
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

using Microsoft::WRL::ComPtr;
using namespace std;

static const wchar_t* TAB = L"  ";

void logOutputDisplayModes(IDXGIOutput* output, wstring indent)
{
	vector<DXGI_MODE_DESC> modes;
	populateOutputDisplayModeList(output, modes);

	for (uint i = 0; i < modes.size(); i++)
	{
		wcout << indent
			<< "Size: " << modes[i].Width << "X" << modes[i].Height
			<< " Refresh: " << modes[i].RefreshRate.Numerator << "/" << modes[i].RefreshRate.Denominator
			<< endl;
	}
}

void logAdapterOutputs(IDXGIAdapter* adapter, wstring indent)
{
	vector<ComPtr<IDXGIOutput>> outputs;
	populateAdapterOutputList(adapter, outputs);
	for (uint i = 0; i < outputs.size(); i++)
	{
		wcout << indent << "* Output " << i << " *" << endl;
		IDXGIOutput* output = outputs.at(i).Get();
		DXGI_OUTPUT_DESC description;
		hrThrowIfFailed(output->GetDesc(&description));
		indent += TAB;
		wcout << indent << description.DeviceName << endl;
		wcout << indent << "Desktop Coordinates: (" 
			<< description.DesktopCoordinates.left   << ", "
			<< description.DesktopCoordinates.top    << ", "
			<< description.DesktopCoordinates.right  << ", "
			<< description.DesktopCoordinates.bottom << ")" << endl;
		wcout << indent << "Display Modes:" << endl;
		logOutputDisplayModes(output, indent + TAB);
	}
}

void logAdapters(vector<ComPtr<IDXGIAdapter>>& adapters)
{
	cout << "Logging " << adapters.size() << " adapters." << endl;
	for (uint i = 0; i < adapters.size(); i++)
	{
		cout << "*** Adapter " << i << " ***" << endl;
		IDXGIAdapter* adapter = adapters.at(i).Get();
		DXGI_ADAPTER_DESC description;
		hrThrowIfFailed(adapter->GetDesc(&description));
		wstring indent = TAB;
		wcout << indent << description.Description << endl;
		wcout << indent << (double)description.DedicatedVideoMemory  / GIGABYTE << " GB of dedicated video memory."  << endl;
		wcout << indent << (double)description.DedicatedSystemMemory / GIGABYTE << " GB of dedicated system memory." << endl;
		wcout << indent << (double)description.SharedSystemMemory    / GIGABYTE << " GB of shared system memory."    << endl;
		
		logAdapterOutputs(adapter, indent + TAB);
	}
}

void run_main()
{
	cout << "Hello!" << endl;
	cout << endl;

	ComPtr<IDXGIFactory> factory;
	initDxgiFactory(factory);
	vector<ComPtr<IDXGIAdapter>> adapters;
	populateAdapterList(factory.Get(), adapters);

	logAdapters(adapters);
	cout << endl;
}

int main()
{	
	run_main();

	if (_CrtDumpMemoryLeaks())
		throw "Memory Leak Detected";

	system("pause");
	return 0;
}