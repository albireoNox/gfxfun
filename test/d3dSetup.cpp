#include "d3dSetup.h"

#include "common.h"
#include <iostream>

using Microsoft::WRL::ComPtr;

void initDxgiFactory(ComPtr<IDXGIFactory> out)
{
	throwIfHrFailed(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)out.GetAddressOf()));
}

void foo()
{
	std::cout << "This is a test." << std::endl;
}