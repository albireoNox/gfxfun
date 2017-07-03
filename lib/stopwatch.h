#pragma once

#include "types.h"

class Stopwatch
{
public:
	Stopwatch();

	void start();
	void reset(); // Will still be in a running state. 
	double elapsedSeconds();

protected:
	int64 tickStart;
	int64 ticksPerSecond;
};