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
	}
}

int run_main()
{
	cout << "Hello!" << endl;

	ComPtr<IDXGIFactory> factory;
	initDxgiFactory(factory);
	vector<ComPtr<IDXGIAdapter>> adapters;
	populateAdapterList(factory.Get(), adapters);
	log_adapters(adapters);

	foo();
	int i;
	cin >> i;

	return i;
}

int main()
{	
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	int i = run_main();

	if (_CrtDumpMemoryLeaks())
		throw "Memory Leak Detected";

	return i;
}