//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "UW2FTimer.h"

UW2FTimer::UW2FTimer()
{
    //ctor
}

UW2FTimer::~UW2FTimer()
{
    //dtor
}

void UW2FTimer::Start()
{
    m_TimeStarted = millis();
    m_IsRunning = true;
}

void UW2FTimer::Stop()
{
    m_TimeStopped = millis();
    if(m_IsRunning)  // if the timer never started, we shouldn't update the total time
    {
      m_TotalTime += m_TimeStopped - m_TimeStarted;
    }
    m_IsRunning = false;
}

void UW2FTimer::Reset()
{
    Stop();
    if(m_TotalTime > 0)
      m_TotalTime_backup = m_TotalTime;
    m_TotalTime = 0;
    m_IsRunning = false;
}

long UW2FTimer::GetIntermediateTime()
{
  if(m_IsRunning)
    return((m_TotalTime + millis() - m_TimeStarted)/1000);
  else
    return m_TotalTime/1000;
}
