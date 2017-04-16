#ifndef _TIMER_H_
#define _TIMER_H_

class Timer
{
public:
	Timer();

	float TotalTime() const;	//Return the total time from the beginning to now. (Unit: Second)
	float DeltaTime() const;	//Return the time from the last frame. (Unit: second)

	void Start();				//Recover from stop
	void Stop();				
	void Reset();				//Reset the timer states and restart
	void Tick();				//Timer update, every frame

private:
	float	m_secsPerCount;		//System related value
	float	m_deltaTime;		//Time frame last frame

	__int64 m_baseTime;			//Starting time of the timer
	__int64 m_stopTime;			//Beginning of the current stop time
	__int64 m_pausedTime;		//Total paused time for now
	__int64	m_currTime;			//Current frame time
	__int64	m_prevTime;			//Last frame time

	bool	m_stopped;
};

#endif