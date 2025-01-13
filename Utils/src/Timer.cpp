#include "Timer.h"

#include <assert.h>

Timer::Timer(bool start)
	: mRunning(false)
{
	if (start)
		Start();
}

template<typename DurationType>
double Timer::GetElapsedTime(const std::chrono::high_resolution_clock::time_point& currentPoint) const
{
	assert(false);
	return 0.0;
}

template<>
double Timer::GetElapsedTime<std::chrono::seconds>(const std::chrono::high_resolution_clock::time_point& currentPoint) const
{
	return std::chrono::duration_cast<std::chrono::duration<double>>(currentPoint - mStartTime).count();
}

template<>
double Timer::GetElapsedTime<std::chrono::milliseconds>(const std::chrono::high_resolution_clock::time_point& currentPoint) const
{
	return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(currentPoint - mStartTime).count();
}

void Timer::Start()
{
	mStartTime = std::chrono::high_resolution_clock::now();
	mRunning = true;
}

void Timer::Stop()
{
	if (mRunning)
	{
		mEndTime = std::chrono::high_resolution_clock::now();
		mRunning = false;
	}
}

void Timer::Reset()
{
	Start();
}

double Timer::Stop_s()
{
	Stop();
	return GetElapsedTime<std::chrono::seconds>(mEndTime);
}

double Timer::Stop_ms()
{
	Stop();
	return GetElapsedTime<std::chrono::milliseconds>(mEndTime);
}

double Timer::Elapsed_s() const
{
	return GetElapsedTime<std::chrono::seconds>(std::chrono::high_resolution_clock::now());
}

double Timer::Elapsed_ms() const
{
	return GetElapsedTime<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now());
}