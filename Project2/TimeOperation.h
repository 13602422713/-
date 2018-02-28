#ifndef __TIMEOPERATION_H
#define __TIMEOPERATION_H
#include <windows.h>
#include <iostream>
typedef enum
{
	STOPWATCH_ON=0,
	STOPWATCH_PAUSE=1,
	STOPWATCH_STOP=2,
}StopWatchState;


typedef enum 
{
	TIMEOUTCLOCK_OFF = 0,
	TIMEOUTCLOCK_ON = 1,
}TimeDectionState;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          

class TimeOperation
{
public:
	TimeOperation();
	~TimeOperation();
	//stopwatch
	bool singularStopwatchRestart();
	bool singularStopwatchPause(SYSTEMTIME &getTime);
	bool singularStopwatchPause(int &ms);
	//timeout detection
	bool TimeOutDectectionSetClock(SYSTEMTIME &setTime);
	bool TimeOutDectectionSetClock(int ms);
	bool singularTimeOutDectectionCheckClock();
	bool multipleTimeOutDectectionCheckClock();
private:
	//stopwatch
	SYSTEMTIME m_cStopwatchOrigin;
	StopWatchState m_eSingleStopwatchState;
	//TimeOut clock
	long int m_cTimeOutDectectionThrel;
	long int m_cTimeOutDectectionStartTime;
	TimeDectionState m_eTimeOutDectectionState;
};


#endif
