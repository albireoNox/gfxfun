#include "stopwatch.h"

#include <windows.h>

Stopwatch::Stopwatch()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&this->ticksPerSecond);
}

void
Stopwatch::start()
{
	QueryPerformanceCounter((LARGE_INTEGER*)&this->tickStart);
}

void
Stopwatch::reset()
{
	this->tickStart = 0;
	start();
}

double Stopwatch::elapsedSeconds()
{
	int64 tickNow;
	QueryPerformanceCounter((LARGE_INTEGER*)&tickNow);
	return (tickNow - this->tickStart) / (double)this->ticksPerSecond;
}
