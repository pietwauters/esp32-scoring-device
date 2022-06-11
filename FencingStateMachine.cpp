//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "FencingStateMachine.h"
#include <iostream>
using namespace std;

#define FIGHTING_MINUTES 3
#define FIGHTING_SECONDS 0

#define BREAK_MINUTES 1
#define BREAK_SECONDS 0

#define ADDITIONAL_TIME_MINUTES 1
#define ADDITIONAL_TIME_SECONDS 0

#define TIME_TO_REARM 10000   //Time in milliseconds

volatile SemaphoreHandle_t timerSemaphore_FSMPeriod;
void IRAM_ATTR onTimer_FSMPeriod()
{
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore_FSMPeriod, NULL);
}


FencingStateMachine::FencingStateMachine(int hw_timer_nr, int tickPeriod)
{
    //ctor

    // Create semaphore to inform us when the timer has fired
    // Semaphore in Counting mode to allow some jitter if some tasks take a bit longer than the FSMPeriod
    timerSemaphore_FSMPeriod = xSemaphoreCreateCounting(10,0);
    // Use 2nd timer of 4 (counted from zero). It seems timer0 is already in use (with a different scaler?)
    // Set 8 divider for prescaler (see ESP32 Technical Reference Manual for more info).
    timer_FSMPeriod = timerBegin(hw_timer_nr, 8, true);
    // Attach onTimer function to our timer.
    timerAttachInterrupt(timer_FSMPeriod, &onTimer_FSMPeriod, true);
    // Set alarm to call onTimer function every x microsecond microseconds.
    // Repeat the alarm (third parameter)
    timerAlarmWrite(timer_FSMPeriod, 10000 * tickPeriod, true);
    // Start an alarm
    timerAlarmEnable(timer_FSMPeriod);
    m_nrOfRounds = 1;
    ResetAll();
    m_Timer.SetTicksPeriod(tickPeriod);
    //m_Timer.SetDisplayResolution(100);
}

FencingStateMachine::~FencingStateMachine()
{
    //dtor
}

void FencingStateMachine::update (MultiWeaponSensor *subject, uint32_t eventtype)
{
  //SetMachineLights(subject->get_Lights());
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;

  if(EVENT_LIGHTS == maineventtype)
      SetMachineLights(eventtype); // I'm doing an implicit type cast from uint32_t to uint8_t
  if(EVENT_WEAPON == maineventtype)
    SetMachineWeapon(subject->GetActualWeapon());
}


void FencingStateMachine::update (UDPIOHandler *subject, uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;;
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  uint32_t card_event;
  m_IsConnectedToRemote = true;

  if(EVENT_UI_INPUT != maineventtype)
    return;
  if(m_Timer.IsRunning())
  {
    if(UI_INPUT_TOGGLE_TIMER == event_data)
    {
      event_data = UI_INPUT_STOP_TIMER;
    }
    if(UI_INPUT_STOP_TIMER != event_data) // Ignore all buttons except stop timer
        return;
  }
  else
  {
    if(UI_INPUT_TOGGLE_TIMER == event_data)
    {
      event_data = UI_INPUT_START_TIMER;
    }
  }

  switch(event_data)
  {

    case UI_INPUT_START_TIMER:
     if(m_Timerstate != MATCH_ENDED)
     {
      m_Timer.StartTimer();
      StateChanged(EVENT_TIMER_STATE | 0x00000001);
      if((FIGHTING == m_Timerstate) && (m_nrOfRounds > 1))  // We only deal with unwillingness to fight in direct eleminiation and team events
      {
        if(m_Timer.GetMinutes() * 60 + m_Timer.GetSeconds() > 60)  // don't start a new period if there is less than a minute left
        {
          m_UW2FTimer.Start();
        }
        else
        {
          if(m_UW2FTimer.GetIntermediateTime() > 0)  // The timer had already been running without reset, so continue
          {
            m_UW2FTimer.Start();
          }

        }
      }
      if(m_TheSensor)
      {
        if(m_NoHitsAllowed)
        {
          m_TheSensor->AllowAllNewHits();
          m_NoHitsAllowed = false;
        }
      }
     }
    break;

    case UI_INPUT_STOP_TIMER:
      m_Timer.StopTimer();
      m_UW2FTimer.Stop();
      StateChanged(EVENT_TIMER_STATE );

    break;

    case UI_INPUT_RESET:
    ResetAll();

    break;

    case UI_INPUT_INCR_SCORE_LEFT:
    m_ScoreLeft++;
    StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
    break;

    case UI_INPUT_INCR_SCORE_RIGHT:
    m_ScoreRight++;
    StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
    break;

    case UI_INPUT_DECR_SCORE_LEFT:
    if(m_ScoreLeft)
    {
      m_ScoreLeft--;
      StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
    }
    break;

    case UI_INPUT_DECR_SCORE_RIGHT:
    if(m_ScoreRight)
    {
      m_ScoreRight--;
      StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
    }
    break;

    case UI_INPUT_PRIO:
    if(m_Priority != NO_PRIO)
    {
      m_Priority = NO_PRIO;
      StateChanged(EVENT_PRIO);

    }
    else
    {
      uint32_t rnd = millis();
      if(rnd & 1)
      {
        m_Priority = PRIO_LEFT;
        StateChanged(EVENT_PRIO | 1);
      }

      else
      {
        m_Priority = PRIO_RIGHT;
        StateChanged(EVENT_PRIO | 2);
      }
    }


    break;

    case UI_INPUT_ROUND:

    switch(m_nrOfRounds)
    {
      case 1:
      m_nrOfRounds = 3;
      break;

      case 3:
      m_nrOfRounds = 9;
      break;

      default:
      m_nrOfRounds = 1;
      break;


    }
    m_currentRound = 1;

    StateChanged(EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);

    break;

    case UI_INPUT_YELLOW_CARD_LEFT:
      if(m_RedCardLeft == 0)
      {
        m_YellowCardLeft=1;
        card_event = m_YellowCardLeft | EVENT_YELLOW_CARD_LEFT;
        StateChanged(card_event );
      }

    break;

    case UI_INPUT_YELLOW_CARD_RIGHT:
      if(m_RedCardRight == 0)
      {
        m_YellowCardRight = 1;
        card_event = m_YellowCardRight | EVENT_YELLOW_CARD_RIGHT;
        StateChanged(card_event );
      }

    break;

    case UI_INPUT_RED_CARD_LEFT:
      m_RedCardLeft++;
      card_event = m_RedCardLeft | EVENT_RED_CARD_LEFT;
      StateChanged(card_event );
      m_UW2FTimer.Reset();
      StateChanged(EVENT_UW2F_TIMER);
      m_ScoreRight++;
      StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
    break;

    case UI_INPUT_RED_CARD_RIGHT:
      m_RedCardRight++;
      card_event = m_RedCardRight | EVENT_RED_CARD_RIGHT;
      StateChanged(card_event );
      m_UW2FTimer.Reset();
      StateChanged(EVENT_UW2F_TIMER);
      m_ScoreLeft++;
      StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
    break;


    case UI_INPUT_P_CARD:
    ProcessUW2F();
    StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
    m_UW2FTimer.Reset();
    StateChanged(EVENT_UW2F_TIMER);
    break;

    case UI_BUZZ:
    StateChanged(EVENT_TOGGLE_BUZZER);
    break;

    case UI_INPUT_RESTORE_UW2F_TIMER:

    m_UW2FTimer.RestorePreviousState();
    m_UW2FSeconds = m_UW2FTimer.GetIntermediateTime();
    StateChanged(EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);

    break;



    case UI_INPUT_CYCLE_WEAPON:

    switch(m_MachineWeapon)
    {
      case SABRE:
      m_MachineWeapon = FOIL;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_FOIL);
      m_TheSensor->SetActualWeapon(FOIL);
      m_TheSensor->Setweapon_detection_mode(MANUAL);
      break;

      case FOIL:
      m_MachineWeapon = EPEE;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_EPEE);
      m_TheSensor->SetActualWeapon(EPEE);
      m_TheSensor->Setweapon_detection_mode(MANUAL);

      break;

      case EPEE:
      m_MachineWeapon = UNKNOWN;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_UNKNOWN);
      m_TheSensor->SetActualWeapon(EPEE);
      m_TheSensor->Setweapon_detection_mode(AUTO);

      break;


      case UNKNOWN:
      m_MachineWeapon = SABRE;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_SABRE);
      m_TheSensor->SetActualWeapon(SABRE);
      m_TheSensor->Setweapon_detection_mode(MANUAL);

      break;

      default:
      m_MachineWeapon = EPEE;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_EPEE);
      m_TheSensor->SetActualWeapon(EPEE);
      m_TheSensor->Setweapon_detection_mode(AUTO);

    }

    break;


  }

}

void FencingStateMachine::ProcessUW2F()
{
    if(m_ScoreLeft != m_ScoreRight)
    {
        if(m_ScoreLeft > m_ScoreRight)
        {
          if(m_PCardRight < 3)
          {
            m_PCardRight++;
            if(m_PCardRight > 1)
            {
              m_ScoreLeft++;
              StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
            }
          }
        }

        else
        {
          if(m_PCardLeft < 3)
          {
            m_PCardLeft++;
            if(m_PCardLeft > 1)
            {
              m_ScoreRight++;
              StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
            }
          }
        }

    }
    else
    {

      if(m_PCardLeft < 3)
      {
        m_PCardLeft++;
        if(m_PCardLeft > 1)
        {
          m_ScoreRight++;
          StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
        }
      }
      if(m_PCardRight < 3)
      {
        m_PCardRight++;
        if(m_PCardRight > 1)
        {
          m_ScoreLeft++;
          StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
        }
      }

    }
}

uint32_t FencingStateMachine::MakeTimerEvent()
{
  uint32_t temp = (m_Timer.GetMinutes()<<16 | m_Timer.GetSeconds()<<8  | m_Timer.GetHundredths());

  temp |= EVENT_TIMER;
  return temp;
}
void FencingStateMachine::ClearAllCards()
{
  m_YellowCardLeft =0;
  m_YellowCardRight =0;
  m_RedCardLeft =0;
  m_RedCardRight =0;
  m_BlackCardLeft =0;
  m_BlackCardRight =0;
  m_YellowPCardLeft =0;
  m_YellowPCardRight =0;
  m_RedPCardLeft =0;
  m_RedPCardRight =0;
  m_BlackPCardLeft =0;
  m_BlackPCardRight =0;
  m_PCardLeft = 0;
  m_PCardRight = 0;
  StateChanged(EVENT_P_CARD);
  StateChanged(EVENT_YELLOW_CARD_RIGHT);
  StateChanged(EVENT_RED_CARD_RIGHT);
  StateChanged(EVENT_BLACK_CARD_RIGHT);
  StateChanged(EVENT_YELLOW_CARD_LEFT);
  StateChanged(EVENT_RED_CARD_LEFT);
  StateChanged(EVENT_BLACK_CARD_LEFT);

}
void FencingStateMachine::ResetAll()
{
     m_Timer.StopTimer();
     m_UW2FTimer.Stop();
     m_UW2FTimer.Reset();
     StateChanged(EVENT_TIMER_STATE);
     StateChanged(EVENT_UW2F_TIMER);
     m_Timer.SetMinutes(FIGHTING_MINUTES);
     m_Timer.SetSeconds(FIGHTING_SECONDS);
     m_Timerstate = FIGHTING;
     StateChanged(MakeTimerEvent());
     m_currentRound = 1;
     //m_nrOfRounds = 1;   // a "simple reset should not change the nr of rounds"
     m_Priority = NO_PRIO;
     ClearAllCards();
     m_ScoreLeft =0;
     m_ScoreRight =0;

     //StateChanged(EVENT_LIGHTS | m_Lights);
     StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
     StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
     StateChanged(EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);
     StateChanged(EVENT_PRIO);
     StateChanged(EVENT_RED_CARD_RIGHT);
     StateChanged(EVENT_YELLOW_CARD_RIGHT);
     StateChanged(EVENT_RED_CARD_LEFT);
     StateChanged(EVENT_YELLOW_CARD_LEFT);
     StateChanged(EVENT_P_CARD);

     if(m_TheSensor)
     {
        m_TheSensor->AllowAllNewHits();
        m_NoHitsAllowed = false;
     }

}

bool FencingStateMachine::ShouldBlockHitsOnTimerZero()
{
  switch(m_Timerstate )
  {
    case FIGHTING :
    case ADDITIONAL_MINUTE :
    return true;
    break;

    default:
    return false;
  }

}
//enum TimerState_t{FIGHTING, BREAK, INJURY, ADDITIONAL_MINUTE, UNDEFINED};

void FencingStateMachine::SetNextTimerStateAndRoundAndNewTimeOnTimerZero()
{
  switch(m_Timerstate )
  {
    case FIGHTING :
      if(m_currentRound < m_nrOfRounds)
      {
        if(9 != m_nrOfRounds)
        {
          m_Timerstate = BREAK;
          m_Timer.SetMinutes(BREAK_MINUTES);
          m_Timer.SetSeconds(BREAK_SECONDS);
          m_UW2FTimer.Reset();
          m_UW2FSeconds = 0;
          StateChanged(EVENT_UW2F_TIMER);
        }
        else
        {
          m_Timer.SetMinutes(FIGHTING_MINUTES);
          m_Timer.SetSeconds(FIGHTING_SECONDS);
          m_UW2FTimer.Reset();
          m_UW2FSeconds = 0;
          StateChanged(EVENT_UW2F_TIMER);
          ClearAllCards();
          m_currentRound++;
        }
      }
      else
      {
        if(m_ScoreLeft == m_ScoreRight)
        {
          m_Timerstate = ADDITIONAL_MINUTE;
          m_Timer.SetMinutes(ADDITIONAL_TIME_MINUTES);
          m_Timer.SetSeconds(ADDITIONAL_TIME_SECONDS);
          m_currentRound = 255;
          StateChanged(MakeTimerEvent());
          m_UW2FTimer.Reset();
          m_UW2FSeconds = 0;
          StateChanged(EVENT_UW2F_TIMER);

          // here I could add the automatic priority determination
        }
        else
        {
          m_Timerstate = MATCH_ENDED;
        }
      }
    break;

    case BREAK :
      m_Timerstate = FIGHTING;
      m_Timer.SetMinutes(FIGHTING_MINUTES);
      m_Timer.SetSeconds(FIGHTING_SECONDS);
      m_UW2FTimer.Reset();
      m_UW2FSeconds = 0;
      StateChanged(EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);

      if(m_currentRound < m_nrOfRounds)
        m_currentRound++;
    break;

    case ADDITIONAL_MINUTE :
    m_Timerstate = MATCH_ENDED;
    break;


    default:  // keep state unchanged
    break;
  }

}

// As a minimum we should perform a timer tick
char ChronoString[32];
int RestartTimerTime;
void FencingStateMachine::DoStateMachineTick()
{
  bool idle = true;
  while (!xSemaphoreTake(timerSemaphore_FSMPeriod, 1 / (portTICK_PERIOD_MS)) == pdTRUE); // check with 1 ms timeout
  //while(xSemaphoreTake(timerSemaphore_FSMPeriod, 0) == pdTRUE)
  {
    if(m_NoHitsAllowed)
    {
        if(millis() > m_timeToRearm)
        {
          m_TheSensor->AllowAllNewHits();
          m_NoHitsAllowed = false;
        }
    }
    if(m_Timer.DoTick())
    {
        // So something interesting because the timer changed
        m_Timer.GetFormattedStringTime(ChronoString, 1, 1);
        idle = false;
        if(m_Timer.ReachedZero())
        {
          if(m_TheSensor)
          {
            if(ShouldBlockHitsOnTimerZero())
            {
              m_TheSensor->BlockAllNewHits();
              m_NoHitsAllowed = true;
              m_timeToRearm = millis() + TIME_TO_REARM;
            }
          }
            // we should do something to send only an event on change;
            m_Timer.StopTimer();
            StateChanged(EVENT_TIMER_STATE);
            SetNextTimerStateAndRoundAndNewTimeOnTimerZero();
            StateChanged(EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);
        }

       StateChanged(MakeTimerEvent());
    }
  }
  if(m_UW2FTimer.IsRunning())
  {
      if(m_UW2FSeconds != m_UW2FTimer.GetIntermediateTime())
      {
          m_UW2FSeconds = m_UW2FTimer.GetIntermediateTime();
          StateChanged(EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);
      }
  }

  if(m_LightsChanged)
  {
    m_LightsChanged = false;
    idle = false;
    if((FIGHTING == m_Timerstate) || (ADDITIONAL_MINUTE == m_Timerstate))
    {
      if(GetRed() || GetGreen())
      {
        if(m_Timer.IsRunning())
        {
          m_UW2FTimer.Reset();
          StateChanged(EVENT_UW2F_TIMER);
        }
        m_Timer.StopTimer();
        StateChanged(EVENT_TIMER_STATE);

        // Make sure you show the the score for a bit longer than normal

      }
      if(m_MachineWeapon == FOIL)
      {
        if(GetWhiteL() || GetWhiteR())
        {
          if(m_Timer.IsRunning())
          {
            m_UW2FTimer.Reset();
            StateChanged(EVENT_UW2F_TIMER);
          }
          m_Timer.StopTimer();
          StateChanged(EVENT_TIMER_STATE);


        }
      }
    }
    StateChanged(EVENT_LIGHTS | m_Lights);

  }


  if(m_WeaponChanged)
  {
    m_WeaponChanged = false;
    idle = false;
    switch(m_MachineWeapon)
    {
      case FOIL:
        StateChanged(EVENT_WEAPON | WEAPON_MASK_FOIL);
      break;

      case EPEE:
        StateChanged(EVENT_WEAPON | WEAPON_MASK_EPEE);
      break;

      case SABRE:
        StateChanged(EVENT_WEAPON | WEAPON_MASK_SABRE);
      break;

      default:
       StateChanged(EVENT_WEAPON | WEAPON_MASK_UNKNOWN);

    }

  }

}
