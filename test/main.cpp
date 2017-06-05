#include "common.h"
#include "d3dSetup.h"
#include <iostream>

using Microsoft::WRL::ComPtr;

int main()
{	
	std::cout << "Hello!" << std::endl;

	ComPtr<IDXGIFactory> factory;
	initDxgiFactory(factory);

	foo();
	int i;
	std::cin >> i;

	if (_CrtDumpMemoryLeaks())
		throw "Memory Leak Detected";

	return i;
}