//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "FencingTimer.h"
#include <iostream>
//using namespace std;
FencingTimer::FencingTimer()
{
    //ctor

    m_ResolutionCounter = m_DisplayResolution;
    MakeNewString();
}

FencingTimer::~FencingTimer()
{
    //dtor
}

void FencingTimer::MakeNewString()
{
    if((m_Minutes == 0) && (m_Seconds < 10))
       {
           sprintf(m_TimerString,"%u.%02u",m_Seconds,m_Hundredths);
       }
       else
        {
            sprintf(m_TimerString,"%u:%02u",m_Minutes,m_Seconds);
        }
}

void FencingTimer::GetFormattedStringTime(char *Destination, int MinutePrecision, int HundredthsPrecision)
{
// Minutes can have 1 or 2 digits
// Seconds always have 2 digits
// Hundredths can have 0, 1 or 2 digits
char temp[8];

    if(m_Minutes > 9) // I need 2 digits regardless of the minuteprecision
    {
        sprintf(Destination,"%02u:%02u",m_Minutes,m_Seconds);
    }

    else
    {
        if(MinutePrecision > 1)
            sprintf(Destination,"%02u:%02u",m_Minutes,m_Seconds);
        else
            sprintf(Destination,"%1u:%02u",m_Minutes,m_Seconds);
    }


    if(HundredthsPrecision > 0)
    {

        if(HundredthsPrecision == 1)
        {
            sprintf(temp,".%u",(m_Hundredths)/10);
        }
        else
        {
            sprintf(temp,".%02u",m_Hundredths);
        }
        strcat(Destination,temp);
    }
}

bool FencingTimer::DoTick()
{
    if(!m_TimerIsRunning)
        return false;
    m_Ticks++;
    if(m_Ticks < m_TicksInOneHundreth)
        return false;
    m_Ticks = 1;
    bool returnvalue = false;
    if(m_Hundredths > 0)
    {
        m_Hundredths--;
        if(m_ShowingHundredths)
        {
            m_ResolutionCounter--;
            if(!m_ResolutionCounter)
            {
                MakeNewString();
                m_ResolutionCounter = m_DisplayResolution;
                returnvalue = true;
            }

        }

    }
    else
    {
        returnvalue = true;

        if(m_Seconds > 0)
        {
            m_Seconds--;
            m_Hundredths = 99;
            if((m_Minutes == 0) && (m_Seconds < 10))
            {
                m_ShowingHundredths = true;
                return false;
            }

            else
                m_ShowingHundredths = false;
        }
        else
        {

            if(m_Minutes > 0)
            {
                m_Minutes--;
                m_Seconds=59;
                m_Hundredths = 99;
            }

            else
            {
                m_TimerIsRunning = false;
                return true;
            }
            MakeNewString();
            m_Seconds = 59;
            return true;

        }
        MakeNewString();

    }
    return returnvalue;
}
