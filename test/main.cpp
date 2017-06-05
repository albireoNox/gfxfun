#include "common.h"
#include "d3dSetup.h"
#include <iostream>
#include <vector>

using Microsoft::WRL::ComPtr;

int run_main()
{
	std::cout << "Hello!" << std::endl;

	ComPtr<IDXGIFactory> factory;
	initDxgiFactory(factory);
	std::vector<ComPtr<IDXGIAdapter>> adapters;
	populateAdapterList(factory.Get(), adapters);

	foo();
	int i;
	std::cin >> i;

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