#include "stdafx.h"
#include "Timer.h"
#include <Windows.h>

Timer::Timer():m_deltaTime(0.f),
	m_baseTime(0),
	m_stopTime(0),
	m_pausedTime(0),
	m_currTime(0),
	m_prevTime(0),
	m_stopped(false)
{
	__int64 frequency;
	//Initialize system related parameter
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));
	m_secsPerCount = 1.f/frequency;
}

float Timer::TotalTime() const
{
	//Stopping state:
	//	Start T0				Stoped S   Current T
	//		|---------------------|------------|
	//TotalTime: S - T0 - TotalPausedTime
	if(m_stopped)
	{
		return (m_stopTime-m_baseTime-m_pausedTime)*m_secsPerCount;
	}
	//Normal state:
	//	 Start T0							Current T
	//		|----------------------------------|
	//TotalTime£ºT - T0 - TotalPausedTime
	else
	{
		return (m_currTime-m_baseTime-m_pausedTime)*m_secsPerCount;
	}
}

float Timer::DeltaTime() const
{
	return m_deltaTime;
}

void Timer::Start()
{
	//Restart
	if(m_stopped)
	{
		__int64 startTime;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&startTime));

		m_pausedTime += (startTime-m_stopTime);
		m_prevTime = startTime;
		m_stopTime = 0;
		m_stopped = false;
	}
}

void Timer::Stop()
{
	if(!m_stopped)
	{
		__int64 curr;
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curr));
		m_stopTime = curr;
		m_stopped = true;
	}
}

void Timer::Reset()
{
	__int64 curr;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curr));
	
	m_baseTime = curr;
	m_currTime = curr;
	m_prevTime = curr;
	m_deltaTime = 0.f;

	m_stopTime = 0;
	m_pausedTime = 0;
	m_stopped = false;
}

void Timer::Tick()
{
	//Per frame update
	//If stopped, do nothing
	if(m_stopped)
	{
		m_deltaTime = 0.f;
		return;
	}

	__int64 curr;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&curr));
	m_currTime = curr;
	
	m_deltaTime = (m_currTime-m_prevTime)*m_secsPerCount;
	m_deltaTime = m_deltaTime<0? 0.f : m_deltaTime;

	m_prevTime = m_currTime;
}