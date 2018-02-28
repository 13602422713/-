#include "TimeOperation.h"

using namespace std;

TimeOperation::TimeOperation()
{
	m_eSingleStopwatchState = STOPWATCH_STOP;
	m_eTimeOutDectectionState = TIMEOUTCLOCK_OFF;
}


TimeOperation::~TimeOperation()
{
	
}



/*******************************************************************************
* Function Name  : singleStopwatchRestart
* Description    : restart stopwatch
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool TimeOperation::singularStopwatchRestart()
{
	m_eSingleStopwatchState = STOPWATCH_ON;
	GetLocalTime(&m_cStopwatchOrigin);


	//cout<<"start.wSecond:"<<m_cStopwatchOrigin.wMinute<<endl;
	//cout<<"start.wMilliseconds:"<<m_cStopwatchOrigin.wMilliseconds<<endl;

	return true;
}
/*******************************************************************************
* Function Name  : singleStopwatchPause
* Description    : pause stopwatch and get time (In Milliseconds).
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool TimeOperation::singularStopwatchPause(int &ms)
{
	if(STOPWATCH_ON != m_eSingleStopwatchState )
	{
		return false;
	}

	SYSTEMTIME EndTime;
	GetLocalTime(&EndTime);

	//cout<<"end.wSecond:"<<EndTime.wMinute<<endl;
	//cout<<"end.wMilliseconds:"<<EndTime.wMilliseconds<<endl;

	ms = EndTime.wMilliseconds-m_cStopwatchOrigin.wMilliseconds + ((EndTime.wSecond-m_cStopwatchOrigin.wSecond) + (EndTime.wMinute - m_cStopwatchOrigin.wMinute)*60)*1000;

	cout<<"ms:"<<ms<<endl;

	m_eSingleStopwatchState = STOPWATCH_PAUSE;

	return true;
}
/*******************************************************************************
* Function Name  : singleStopwatchPause
* Description    : pause stopwatch and get time (In Milliseconds).
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool TimeOperation::TimeOutDectectionSetClock(SYSTEMTIME &setTime)
{
	m_cTimeOutDectectionThrel = ((setTime.wHour*60+setTime.wMinute)*60+setTime.wSecond)*1000+setTime.wMilliseconds;

	//set now time
	SYSTEMTIME nowTime;
	GetLocalTime(&nowTime);
	m_cTimeOutDectectionStartTime = ((nowTime.wHour*60+nowTime.wMinute)*60+nowTime.wSecond)*1000+nowTime.wMilliseconds;

	m_eTimeOutDectectionState = TIMEOUTCLOCK_ON;
	return true;
}
/*******************************************************************************
* Function Name  : TimeOutDectectionSetClock
* Description    : pause stopwatch and get time (In Milliseconds).
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool TimeOperation::TimeOutDectectionSetClock(int ms)
{
	m_cTimeOutDectectionThrel = ms;

	SYSTEMTIME nowTime;
	GetLocalTime(&nowTime);
	m_cTimeOutDectectionStartTime = ((nowTime.wHour*60+nowTime.wMinute)*60+nowTime.wSecond)*1000+nowTime.wMilliseconds;

	m_eTimeOutDectectionState = TIMEOUTCLOCK_ON;
	return true;	
}
/*******************************************************************************
* Function Name  : singleStopwatchPause
* Description    : pause stopwatch and get time (In Milliseconds).
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool TimeOperation::singularTimeOutDectectionCheckClock()
{
	if( TIMEOUTCLOCK_OFF == m_eTimeOutDectectionState )
	{
		return false;
	}

	SYSTEMTIME nowTime;
	GetLocalTime(&nowTime);
	long int nowTime_ms=((nowTime.wHour*60+nowTime.wMinute)*60+nowTime.wSecond)*1000+nowTime.wMilliseconds;

	if( nowTime_ms -  m_cTimeOutDectectionStartTime> m_cTimeOutDectectionThrel)
	{
		m_eTimeOutDectectionState = TIMEOUTCLOCK_OFF;
		return true;
	}
	return false;
}
/*******************************************************************************
* Function Name  : singleStopwatchPause
* Description    : pause stopwatch and get time (In Milliseconds).
* Input          : None
* Output         : None
* Return         : succeed/failed
*******************************************************************************/
bool TimeOperation::multipleTimeOutDectectionCheckClock()
{
	if( TIMEOUTCLOCK_OFF == m_eTimeOutDectectionState )
	{
		return false;
	}

	SYSTEMTIME nowTime;
	GetLocalTime(&nowTime);
	long int nowTime_ms=(nowTime.wHour*60+nowTime.wMinute)*1000+nowTime.wMilliseconds;

	if( nowTime_ms > m_cTimeOutDectectionThrel)
	{
		return true;
	}
	return false;
}


