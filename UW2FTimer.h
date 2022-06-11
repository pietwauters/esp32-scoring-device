//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef UW2FTIMER_H
#define UW2FTIMER_H
#include <Arduino.h>
#include <string>

class UW2FTimer
{
    public:
        UW2FTimer();
        virtual ~UW2FTimer();

        bool IsRunning() { return m_IsRunning; }
        std::string Getm_strTime() { return m_strTime; }
        void Start();
        void Stop();
        void Reset();
        void RestorePreviousState(){m_TotalTime = m_TotalTime_backup;};
        long GetIntermediateTime();


    protected:

    private:
        long m_TotalTime =0;
        long m_TotalTime_backup =0;
        long m_TimeStarted =0;
        long m_TimeStopped =0;
        bool m_IsRunning = false;
        long m_seconds =0;
        long m_minutes = 0;
        std::string m_strTime = "";
};

#endif // UW2FTIMER_H
