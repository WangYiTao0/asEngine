#pragma once

#define xFRANK_LUNA_TIMER

#ifdef FRANK_LUNA_TIMER
// Borrowed from Frank Luna's DirectX11 book's GameTimer class
// https://www.amazon.com/Introduction-3D-Game-Programming-DirectX/dp/1936420228

class PerfTimer
{
public:
	PerfTimer();
	~PerfTimer();

	float TotalTime() const;
	double CurrentTime();
	float DeltaTime() const;	// return dt

	void Reset();
	void Start();
	void Stop();
	float Tick();

private:
	double m_secPerCount;
	double m_dt;

	long m_baseTime;
	long m_pausedTime;
	long m_stopTime;
	long m_prevTime;
	long m_currTime;

	bool m_stopped;
};
#else	// C++11

#include <chrono>

using Vtime_t = std::chrono::time_point<std::chrono::system_clock>;	// Vtime_t != std::time_t
using duration_t = std::chrono::duration<float>;

class PerfTimer
{
public:
	PerfTimer();

	// returns the time duration between Start() and Now, minus the paused duration.
	float TotalTime() const;


	//double CurrentTime();

	// returns the last delta time measured between Start() and Stop()
	float DeltaTime() const;
	float GetPausedTime() const;
	float GetStopDuration() const;	// gets (Now - stopTime)
	float GetSeconds() const { return dt.count(); }
	float GetMilliseconds() const { return dt.count() * 1000.0f; }

	void Reset();
	void Start();
	void Stop();
	inline float StopGetDeltaTimeAndReset() { Stop(); float dt = DeltaTime(); Reset(); return dt; }

	// since everything is single threaded, once a timer is started, it has to be updated
	// by calling tick. Tick() will return the time duration since the last time Tick() is called.
	// First call will return the duration between Start() and Tick().
	float Tick();

private:
	Vtime_t		baseTime,
		prevTime,
		currTime,
		startTime,		// pause functionality: start/stop time stamps
		stopTime;
	duration_t	pausedTime,
		dt;
	bool		bIsStopped;
};

#endif
