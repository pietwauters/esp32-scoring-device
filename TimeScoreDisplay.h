//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#ifndef TIMESCOREDISPLAY_H
#define TIMESCOREDISPLAY_H

#include "SubjectObserverTemplate.h"
#include "FencingStateMachine.h"
#include "RepeaterReceiver.h"
#include "EventDefinitions.h"
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Preferences.h>

#ifdef FIRST_PROTO
#define HSPI_MISO   12
#define HSPI_MOSI   13
#define HSPI_SCLK   14
#define HSPI_SS     15
#define CS_PIN    15  // or SS
#endif

#ifdef SECOND_PROTO
#define HSPI_MISO   12
#define HSPI_MOSI   13
#define HSPI_SCLK   14
#define HSPI_SS     27
#define CS_PIN    27  // or SS
#endif

#define TEXT_BRIGHTNESS_LOW 1
#define TEXT_BRIGHTNESS_NORMAL 3
#define TEXT_BRIGHTNESS_HIGH 5
#define TEXT_BRIGHTNESS_ULTRAHIGH 7

enum TimerStatus_t {NO_TIME, // No need to show the time, it's not available
                  TIMER_STOPPED, // Timer stopped, alternate between time and score
                  TIMER_RUNNING, // Timer is running during match, only show timer
                  BREAK_TIME // During a break, you can alternate time and score
};


class TimeScoreDisplay : public Observer<FencingStateMachine>,public Observer<RepeaterReceiver>
{
    public:
        /** Default constructor */
        TimeScoreDisplay();
        /** Default destructor */
        virtual ~TimeScoreDisplay();
        void begin();
        void update (FencingStateMachine *subject, uint32_t eventtype);
        void update (RepeaterReceiver *subject, uint32_t eventtype);
        void ProcessEvents ();
        void DisplayScore(uint8_t scoreLeft, uint8_t scoreRight);
        void DisplayTime(uint8_t minutes, uint8_t seconds, uint8_t hundreths, bool TenthsOnly = true);
        void SetScore(uint8_t scoreLeft, uint8_t scoreRight){m_scoreLeft = scoreLeft; m_scoreRight = scoreRight;};
        void SetTime(uint8_t minutes, uint8_t seconds, uint8_t hundreths){m_minutes = minutes; m_seconds = seconds; m_hundredths = hundreths;};
        void DisplayMatchCount(uint8_t match, uint8_t maxmatch);
        void ClearColumn(uint8_t MostLeftPosition);
        void SetChar(uint8_t MostLeftPosition, uint8_t character);
        void ShowScore() {DisplayScore(m_scoreLeft,m_scoreRight);m_ScoreIsShown = true;};
        void ShowTime() {DisplayTime(m_minutes, m_seconds, m_hundredths);m_ScoreIsShown = false;};
        void AlternateScoreAndTimeWhenNotFighting();
        void CycleScoreMatchAndTimeWhenNotFighting();
        void ShowScoreForGivenDuration(uint32_t duration);
        void DisplayWeapon(weapon_t weapon);
        void DisplayPisteId();
        void SetBrightness(int value);
        static void SetPower(bool bValue){if(bValue){digitalWrite(PowerPin,HIGH);}else{digitalWrite(PowerPin,LOW);}};

    protected:

    private:
    uint8_t m_scoreLeft = 0;
    uint8_t m_scoreRight = 0;
    uint8_t m_minutes = 3;
    uint8_t m_seconds = 0;
    uint8_t m_hundredths = 0;
    uint8_t m_round =  0;
    uint8_t m_maxround = 0;

    TimerStatus_t m_TimerStatus = NO_TIME;
    bool m_separatorshown = true;
    bool m_ScoreIsShown = true;
    uint8_t m_objectshown = 0;
    int previous_seconds=0;
    long NextTimeToSwitchBetweenScoreAndTime = 0;
    long NextTimeToTogglecolon = 0;
    uint32_t m_LastEvent = 0;
    QueueHandle_t queue = NULL;
    int m_Brightness = TEXT_BRIGHTNESS_NORMAL;
    int PisteId = -1;
    bool m_Idle = false;


};

#endif // TIMESCOREDISPLAY_H
