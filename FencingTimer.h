//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef FENCINGTIMER_H
#define FENCINGTIMER_H

#include  <cstdint>
#include <cstdio>
#include <string.h>
class FencingTimer
{
    public:
        /** Default constructor */
        FencingTimer();
        /** Default destructor */
        virtual ~FencingTimer();

        /** Access m_Minutes
         * \return The current value of m_Minutes
         */
        uint8_t GetMinutes() { return m_Minutes; }
        /** Set m_Minutes
         * \param val New value to set
         */
        void SetMinutes(uint8_t val) { m_Minutes = val; m_Ticks=0; m_Hundredths = 100;MakeNewString();}
        /** Access m_Seconds
         * \return The current value of m_Seconds
         */
        uint8_t GetSeconds() { return m_Seconds; }
        /** Set m_Seconds
         * \param val New value to set
         */
        void SetSeconds(uint8_t val) { m_Seconds = val; m_Ticks=0;m_Hundredths = 100;MakeNewString();}
        /** Access m_Hundredths
         * \return The current value of m_Hundredths
         */
        uint8_t GetHundredths() { return m_Hundredths; }
        /** Set m_Hundredths
         * \param val New value to set
         */
        void SetHundredths(uint8_t val) { m_Hundredths = val; }
        /** Access m_TimerString[5]
         * \return The current value of m_TimerString[5]
         */
        char *GetTimerString() { return m_TimerString; }

        /** Access m_TicksPeriod
         * \return The current value of m_TicksPeriod
         */
        unsigned int GetTicksPeriod() { return m_TicksPeriod;}
        /** Set m_TicksPeriod
         * \param val New value to set
         */
        void SetTicksPeriod(unsigned int val) { m_TicksPeriod = val; m_TicksInOneHundreth = 10 / m_TicksPeriod;}

        /** Set SetDisplayResolution
         * \param val New value to set
         */
        void SetDisplayResolution(unsigned int val) { m_DisplayResolution = val; m_ResolutionCounter = val;}

        bool IsRunning(){return m_TimerIsRunning;}

        /** DoTick
         * \return  true if executing results in new string, false otherwise
         */
        bool DoTick();
        void StartTimer(){m_TimerIsRunning = true;}
        char *StopTimer() { m_TimerIsRunning = false; MakeNewString();return m_TimerString; }
        void GetFormattedStringTime(char *Destination, int MinutePrecision, int HundredthsPrecision);
        bool ReachedZero(){return (!(m_Minutes+m_Seconds+m_Hundredths));};

    private:
        /** MakeNewString
         *  Determine Display std::string from internal Minutes, Seconds, Hundredths
         */
        void MakeNewString();


    protected:

    private:
        uint8_t m_Minutes=3; //!< Member variable "m_Minutes"
        uint8_t m_Seconds=0; //!< Member variable "m_Seconds"
        uint8_t m_Hundredths=0; //!< Member variable "m_Hundredths"
        char m_TimerString[16]; //!< Member variable "m_TimerString[16]"
        unsigned int m_Ticks=0; //!< Member variable "m_Ticks"
        unsigned int m_TicksPeriod=1; //!< Member variable "m_TicksPeriod (in hundredths of seconds"
        unsigned int m_TicksInOneHundreth=10;
        uint8_t m_DisplayResolution = 2;
        bool m_TimerIsRunning = false;
        uint8_t m_ResolutionCounter;
        bool m_ShowingHundredths = false;

};

#endif // FENCINGTIMER_H
