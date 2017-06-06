#include <iostream> 

// Enables detection of memory leaks (Debug only).
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;

int main()
 {
	cout << "Hello" << endl;
	system("pause");
	return 0;
 }