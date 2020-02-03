#include "aspch.h"
#include "asTimer.h"
#include "asHelper.h"
#include "System\asPlatform.h"


double asTimer::PCFreq = 0;

__int64 asTimer::CounterStart = 0;

asTimer::asTimer()
{
	if (CounterStart == 0)
		Start();
	record();
}

asTimer::~asTimer()
{

}

void asTimer::Start()
{
	LARGE_INTEGER li;
	if (!QueryPerformanceFrequency(&li))
		asHelper::messageBox("QueryPerformanceFrequence failed!\n");

	PCFreq = double(li.QuadPart) / 1000.0;

	QueryPerformanceFrequency(&li);
	CounterStart = li.QuadPart;
}

double asTimer::TotalTime()
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return double(li.QuadPart - CounterStart) / PCFreq;
}

void asTimer::record()
{
	lastTime = TotalTime();
}

double asTimer::elapsed()
{
	return TotalTime() - lastTime;
}
