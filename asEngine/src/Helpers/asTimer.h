#pragma once

#include "CommonInclude.h"

class asTimer
{
private:
	static double PCFreq;
	static __int64 CounterStart;

	double lastTime;
public:
	asTimer();
	~asTimer();

	static void Start();
	static double TotalTime();

	//start recording
	void record();
	//elapsed time since record()
	double elapsed();
};