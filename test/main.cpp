#include "common.h"
#include "d3dSetup.h"
#include <iostream>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace std;

class Classy
{
public:
	Classy(int i);
	~Classy();
protected: 
	int i;
};

Classy::Classy(int i) : i(i) { }

Classy::~Classy()
{
	cout << "Destroying classy number " << this->i << "." << endl;
}

int main()
{	
	std::cout << "Hello!" << std::endl;

	{
		vector<Classy> * classies = new vector<Classy>();
		classies->push_back(Classy(1));
		classies->push_back(2);
		Classy * classy = new Classy(3);
		classies->push_back(*classy);
		cout << classies->size() << endl;
		delete classy;
		delete classies;
	}

	ComPtr<IDXGIFactory> factory;
	initDxgiFactory(factory);

	foo();
	int i;
	std::cin >> i;

	if (_CrtDumpMemoryLeaks())
		throw "Memory Leak Detected";

	return i;
}