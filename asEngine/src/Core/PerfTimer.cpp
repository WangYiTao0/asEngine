#include "aspch.h"

#include "PerfTimer.h"

#ifdef FRANK_LUNA_TIMER
#include <Windows.h>

#include <string>

PerfTimer::PerfTimer()
	:
	m_secPerCount(0),
	m_dt(-1),
	m_baseTime(0),
	m_pausedTime(0),
	m_prevTime(0),
	m_currTime(0),
	m_stopped(false)
{
	__int64 countsPerSec;	// use this data type for querying - otherwise stack corruption!
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_secPerCount = 1.0 / (double)(countsPerSec);
}


PerfTimer::~PerfTimer()
{}


float PerfTimer::DeltaTime() const
{
	return static_cast<float>(m_dt);
}

// todo: implement
float PerfTimer::GetPausedTime() const { assert(false); }
float PerfTimer::GetStopDuration() const { assert(false); }

void PerfTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_baseTime = currTime;
	m_prevTime = currTime;
	m_stopTime = 0;
	m_stopped = false;
}

// accumulates paused time & starts the timer
void PerfTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (m_stopped)
	{
		m_pausedTime = (startTime - m_stopTime);
		m_prevTime = startTime;
		m_stopTime = 0;
		m_stopped = false;
	}
}

// stops the timer
void PerfTimer::Stop()
{
	if (!m_stopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_stopTime = currTime;
		m_stopped = true;
	}
}

// returns time elapsed since Reset(), not counting paused time in between
float PerfTimer::TotalTime() const
{
	long timeLength = 0;
	if (m_stopped)
	{
		//					TIME
		// Base	  Stop		Start	 Stop	Curr
		//--*-------*----------*------*------|
		//			<---------->
		//			   Paused
		timeLength = (m_stopTime - m_pausedTime) - m_baseTime;
	}
	else
	{
		//					TIME
		// Base			Stop	  Start			Curr
		//--*------------*----------*------------|
		//				 <---------->
		//					Paused
		timeLength = (m_currTime - m_pausedTime) - m_baseTime;
	}

	return static_cast<float>(timeLength * m_secPerCount);
}

double PerfTimer::CurrentTime()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;
	return m_currTime * m_secPerCount;
}

float PerfTimer::Tick()
{
	if (m_stopped)
	{
		m_dt = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_currTime = currTime;
	m_dt = (m_currTime - m_prevTime) * m_secPerCount;

	m_prevTime = m_currTime;
	m_dt = m_dt < 0.0 ? 0.0 : m_dt;		// m_dt => 0
}
#else


PerfTimer::PerfTimer()
	:
	bIsStopped(true)
{
	Reset();
}

Vtime_t GetNow() { return std::chrono::system_clock::now(); }

float PerfTimer::TotalTime() const
{
	duration_t totalTime = duration_t::zero();
	// Base	  Stop		Start	 Stop	   Curr
	//--*-------*----------*------*---------|
	//			<---------->
	//			   Paused
	if (bIsStopped)	totalTime = (stopTime - baseTime) - pausedTime;

	// Base			Stop	  Start			Curr
	//--*------------*----------*------------|
	//				 <---------->
	//					Paused
	else totalTime = (currTime - baseTime) - pausedTime;

	return totalTime.count();
}


float PerfTimer::DeltaTime() const
{
	return dt.count();
}

void PerfTimer::Reset()
{
	baseTime = prevTime = GetNow();
	bIsStopped = true;
	dt = duration_t::zero();
}

void PerfTimer::Start()
{
	if (bIsStopped)
	{
		pausedTime = startTime - stopTime;
		prevTime = GetNow();
		bIsStopped = false;
	}
	Tick();
}

void PerfTimer::Stop()
{
	Tick();
	if (!bIsStopped)
	{
		stopTime = GetNow();
		bIsStopped = true;
	}
}

float PerfTimer::Tick()
{
	if (bIsStopped)
	{
		dt = duration_t::zero();
		return dt.count();
	}

	currTime = GetNow();
	dt = currTime - prevTime;

	prevTime = currTime;
	dt = dt.count() < 0.0f ? dt.zero() : dt;	// make sure dt >= 0 (is this necessary?)

	return dt.count();
}

float PerfTimer::GetPausedTime() const
{
	return pausedTime.count();
}
float PerfTimer::GetStopDuration() const
{
	duration_t stopDuration = GetNow() - stopTime;
	return stopDuration.count();
}

#endif;
