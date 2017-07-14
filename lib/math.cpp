#include "math.h"
#include "common.h"
#include <iostream>
#include <iomanip>

using namespace std;

void
Math::printMx(DirectX::XMFLOAT4X4& mx)
{
	cout << "[" << endl;
	for (uint row = 0; row < 4; row++)
	{
		cout << "  ";
		for (uint col = 0; col < 4; col++)
		{
			cout << setw(6) << mx.m[row][col] << " ";
		}
		cout << endl;
	}
	cout << "]" << endl;
}
