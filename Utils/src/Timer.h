#pragma once

#include <chrono>

/// <summary>
/// Simple timer struct.
/// </summary>
struct Timer
{
public:
	/// <summary>
	/// Constructor initializing a Timer.
	/// </summary>
	/// <param name="start">Whether to start the timer</param>
	Timer(bool start = false);
private:
	/// <summary>
	/// Helper function to retrieve the elapsed time from the passed time point.
	/// </summary>
	/// <typeparam name="DurationType">The return duration type</typeparam>
	/// <param name="currentPoint">The current time point</param>
	/// <returns>The elapsed time in the DurationType</returns>
	template<typename DurationType>
	double GetElapsedTime(const std::chrono::high_resolution_clock::time_point& currentPoint) const;
public:
	/// <summary>
	/// Starts the timer.
	/// </summary>
	void Start();

	/// <summary>
	/// Stops the timer.
	/// </summary>
	void Stop();

	/// <summary>
	/// Resets the timer.
	/// </summary>
	void Reset();

	/// <summary>
	/// Stops the timer and returns the elapsed time in seconds.
	/// </summary>
	/// <returns>The elapsed time in seconds</returns>
	double Stop_s();

	/// <summary>
	/// Stops the timer and returns the elapsed time in milliseconds.
	/// </summary>
	/// <returns>The elapsed time in milliseconds</returns>
	double Stop_ms();

	/// <summary>
	/// Retrieves the elapsed time in seconds.
	/// </summary>
	/// <returns>The elapsed time in seconds</returns>
	double Elapsed_s() const;

	/// <summary>
	/// Retrieves the elapsed time in milliseconds.
	/// </summary>
	/// <returns>The elapsed time in milliseconds</returns>
	double Elapsed_ms() const;
private:
	bool mRunning;

	std::chrono::high_resolution_clock::time_point mStartTime;
	std::chrono::high_resolution_clock::time_point mEndTime;
};