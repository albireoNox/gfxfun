#include "common.h"
#include "d3dSetup.h"
#include <iostream>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace std;

void log_adapters(vector<ComPtr<IDXGIAdapter>> & adapters)
{
	cout << "Logging " << adapters.size() << " adapters." << endl;
	for (uint i = 0; i < adapters.size(); i++)
	{
		cout << "*** Adapter " << i << " ***" << endl;
		IDXGIAdapter * adapter = adapters.at(i).Get();
		DXGI_ADAPTER_DESC description;
		hrThrowIfFailed(adapter->GetDesc(&description));
		wcout << description.Description << endl;
		wcout << (double)description.DedicatedVideoMemory  / GIGABYTE << " GB of dedicated video memory."  << endl;
		wcout << (double)description.DedicatedSystemMemory / GIGABYTE << " GB of dedicated system memory." << endl;
		wcout << (double)description.SharedSystemMemory    / GIGABYTE << " GB of shared system memory."    << endl;
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

	log_adapters(adapters);
	cout << endl;
}

int main()
{	
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	run_main();

	if (_CrtDumpMemoryLeaks())
		throw "Memory Leak Detected";

	system("pause");
	return 0;
}