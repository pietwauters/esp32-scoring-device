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
    m_TotalTime += m_TimeStopped - m_TimeStarted;
    m_IsRunning = false;
}

void UW2FTimer::Reset()
{
    m_TotalTime = 0;
    m_IsRunning = false;
}

long UW2FTimer::GetIntermediateTime()
{
    return((m_TotalTime + millis() - m_TimeStarted)/1000);
}
