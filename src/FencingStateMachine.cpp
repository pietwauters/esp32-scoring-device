//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "FencingStateMachine.h"
#include <iostream>
#include <Preferences.h>
#include "esp_task_wdt.h"
#include "esp_log.h"
static const char* FSM_TAG = "Fencing State Machine:";
// using namespace std;

#define FIGHTING_MINUTES 3
#define FIGHTING_SECONDS 0

#define BREAK_MINUTES 1
#define BREAK_SECONDS 0

#define ADDITIONAL_TIME_MINUTES 1
#define ADDITIONAL_TIME_SECONDS 0

#define TIME_TO_REARM 10000   //Time in milliseconds
#define IDLE_TIME_MS 1000*66*3   // 3 minute in milliseconds
#define TIME_TO_DEEP_SLEEP 1000*66*6   // 6 minutes in milliseconds


volatile SemaphoreHandle_t timerSemaphore_FSMPeriod;
void IRAM_ATTR onTimer_FSMPeriod()
{
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore_FSMPeriod, NULL);
}

TaskHandle_t StateMachineTask;
void StateMachineHandler(void *parameter)
{
  FencingStateMachine &MyLocalStatemachine= FencingStateMachine::getInstance();
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xInterval = pdMS_TO_TICKS(10); // 10 ms interval
  while(true)
  {
    vTaskDelayUntil(&xLastWakeTime, xInterval); // Maintains periodicity
    MyLocalStatemachine.DoStateMachineTick();
    esp_task_wdt_reset();

  }
}


FencingStateMachine::FencingStateMachine(int hw_timer_nr, int tickPeriod)
{
    //ctor

    // Create semaphore to inform us when the timer has fired
    // Semaphore in Counting mode to allow some jitter if some tasks take a bit longer than the FSMPeriod
//timerSemaphore_FSMPeriod = xSemaphoreCreateCounting(10,0);
    // Use 2nd timer of 4 (counted from zero). It seems timer0 is already in use (with a different scaler?)
    // Set 8 divider for prescaler (see ESP32 Technical Reference Manual for more info).
//timer_FSMPeriod = timerBegin(1000);// set timer at 100Hz
    // Attach onTimer function to our timer.
//    timerAttachInterrupt(timer_FSMPeriod, &onTimer_FSMPeriod);
    // Set alarm to call onTimer function every x microsecond microseconds.
    // Repeat the alarm (third parameter)
//    timerAlarm(timer_FSMPeriod, 10, true,0);
    // Start an alarm
    //timerAlarmEnable(timer_FSMPeriod);
    m_nrOfRounds = 1;
    ResetAll();
    m_Timer.SetTicksPeriod(tickPeriod);
    //m_Timer.SetDisplayResolution(100);
    m_low_prio_divider = m_low_prio_divider_init;


}

void FencingStateMachine::begin()
{
  if(m_HasBegun)
    return;
    xTaskCreatePinnedToCore(
              StateMachineHandler,        /* Task function. */
              "StateMachineHandler",      /* String with name of task. */
              32768,                            /* Stack size in words. 65535*/
              NULL,                             /* Parameter passed as input of the task */
              5,                                /* Priority of the task. */
              &StateMachineTask,           /* Task handle. */
              1);
    esp_task_wdt_add(StateMachineTask);

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

void FencingStateMachine::update (CyranoHandler *subject, std::string eventtype)
{

  EFP1Message input(eventtype);
  ProcessDisplayMessage (input);
}


void FencingStateMachine::ProcessSpecialSetting (uint32_t eventtype)
{
  uint32_t event_data = eventtype & SPECIAL_SETTING_DATA_MASK;
  uint32_t eventsubtype = eventtype & SPECIAL_SETTING_SUBTYPE_MASK ;
  if(UI_SET_PISTE_NR == eventsubtype)
  {
    Preferences networkpreferences;
    networkpreferences.begin("credentials", false);
    networkpreferences.putInt("U", event_data);
    networkpreferences.end();
  }


}

void FencingStateMachine::TransmitFullStateToDisplay (RepeaterSender *TheRepeater){

  //StateChanged(EVENT_LIGHTS | m_Lights);   // Wrong, this will cause everything to transmit
  TheRepeater->update(this,EVENT_LIGHTS | m_Lights);
  m_low_prio_divider--;
  if(!m_low_prio_divider){
    m_low_prio_divider = m_low_prio_divider_init;

  }
  else
    return;
  yield();
  if(m_Timer.IsRunning())
    return;

  TheRepeater->update(this,EVENT_SCORE_LEFT | m_ScoreLeft);

  TheRepeater->update(this,m_YellowCardLeft | EVENT_YELLOW_CARD_LEFT );

  TheRepeater->update(this,m_RedCardLeft | EVENT_RED_CARD_LEFT );

  TheRepeater->update(this,EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
  yield();

  TheRepeater->update(this,EVENT_SCORE_RIGHT | m_ScoreRight);

  TheRepeater->update(this,m_YellowCardRight | EVENT_YELLOW_CARD_RIGHT );

  TheRepeater->update(this,m_RedCardRight | EVENT_RED_CARD_RIGHT );
  yield();

  TheRepeater->update(this,EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);

  TheRepeater->update(this,MakeTimerEvent());

  //TheRepeater->update(this,EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);
}

void FencingStateMachine::update (CyranoHandler *subject, uint32_t eventtype){
  if(EVENT_CYRANO_STATE_LOCKED == eventtype){
      m_UI_State = LOCKED;
      return;}
    if(EVENT_CYRANO_STATE_UNLOCKED == eventtype){
      m_UI_State = UNLOCKED;
    }
}

void FencingStateMachine::update (UDPIOHandler *subject, uint32_t eventtype)
{
  uint32_t event_data = eventtype & UI_SUB_TYPE_MASK;

  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  uint32_t card_event;
  m_IsConnectedToRemote = true;
  if(EVENT_UI_INPUT_SPECIAL_SETTINGS == maineventtype)
    ProcessSpecialSetting(eventtype);
  if(m_UI_State == LOCKED){
      // Limit UI reactions
      if(UI_INPUT_RESET == event_data)
        ResetAll();
      // I should probably check the rounds settings as well
      return;
    }
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

  uint8_t val = (eventtype & 0x0000ff00)>>8;
  switch(eventtype & 0x000000ff){

    case UI_SET_MINUTES:
      m_Timer.SetMinutes(val);
      StateChanged(MakeTimerEvent());

    break;

    case UI_SET_SECONDS:
    m_Timer.SetSeconds(val);

    StateChanged(MakeTimerEvent());
    break;

    case UI_SET_HUNDREDS:
    m_Timer.SetHundredths(val);

    StateChanged(MakeTimerEvent());
    break;
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
    //m_ScoreLeft++;
    incrementScoreAndCheckForMinuteBreak(true);
    StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
    break;

    case UI_INPUT_INCR_SCORE_RIGHT:
    //m_ScoreRight++;
    incrementScoreAndCheckForMinuteBreak(false);
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
      m_nrOfRounds = 2;
      break;

      case 2:
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

    case UI_NEXT_PERIOD:
      SetNextTimerStateAndRoundAndNewTimeOnTimerZero();
      StateChanged(EVENT_TIMER_STATE);
      StateChanged(EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);
      StateChanged(MakeTimerEvent());
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
      m_UW2FSeconds = 0;
      StateChanged(EVENT_UW2F_TIMER);
      //m_ScoreRight++;
      incrementScoreAndCheckForMinuteBreak(false);
      StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
    break;

    case UI_INPUT_RED_CARD_RIGHT:
      m_RedCardRight++;
      card_event = m_RedCardRight | EVENT_RED_CARD_RIGHT;
      StateChanged(card_event );
      m_UW2FTimer.Reset();
      m_UW2FSeconds = 0;
      StateChanged(EVENT_UW2F_TIMER);
      //m_ScoreLeft++;
      incrementScoreAndCheckForMinuteBreak(true);;
      StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
    break;

    case UI_INPUT_BLACK_CARD_LEFT:
        m_BlackCardLeft = 1;
        card_event = m_BlackCardLeft | EVENT_BLACK_CARD_LEFT;
        StateChanged(card_event );

    break;

    case UI_INPUT_BLACK_CARD_RIGHT:
        m_BlackCardRight = 1;
        card_event = m_BlackCardRight | EVENT_BLACK_CARD_RIGHT;
        StateChanged(card_event );

    break;

    case UI_INPUT_YELLOW_CARD_LEFT_DECR:
      if(m_YellowCardLeft == 1)
      {
        m_YellowCardLeft=0;
        card_event = m_YellowCardLeft | EVENT_YELLOW_CARD_LEFT;
        StateChanged(card_event );
      }

    break;

    case UI_INPUT_YELLOW_CARD_RIGHT_DECR:
      if(m_YellowCardRight == 1)
      {
        m_YellowCardRight = 0;
        card_event = m_YellowCardRight | EVENT_YELLOW_CARD_RIGHT;
        StateChanged(card_event );
      }

    break;

    case UI_INPUT_RED_CARD_LEFT_DECR:
      if(m_RedCardLeft)
      {
        m_RedCardLeft--;
        card_event = m_RedCardLeft | EVENT_RED_CARD_LEFT;
        StateChanged(card_event );
        //m_UW2FTimer.Reset();
        //StateChanged(EVENT_UW2F_TIMER);
        m_ScoreRight--;
        StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
      }

    break;

    case UI_INPUT_RED_CARD_RIGHT_DECR:
      if(m_RedCardRight)
      {
        m_RedCardRight--;
        card_event = m_RedCardRight | EVENT_RED_CARD_RIGHT;
        StateChanged(card_event );
        //m_UW2FTimer.Reset();
        //StateChanged(EVENT_UW2F_TIMER);
        m_ScoreLeft--;
        StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
      }

    break;

    case UI_INPUT_BLACK_CARD_LEFT_DECR:
        m_BlackCardLeft = 0;
        card_event = m_BlackCardLeft | EVENT_BLACK_CARD_LEFT;
        StateChanged(card_event );

    break;

    case UI_INPUT_BLACK_CARD_RIGHT_DECR:
        m_BlackCardRight = 0;
        card_event = m_BlackCardRight | EVENT_BLACK_CARD_RIGHT;
        StateChanged(card_event );

    break;

    case UI_INPUT_P_CARD:
    ProcessUW2F();
    StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
    m_UW2FTimer.Reset();
    StateChanged(EVENT_UW2F_TIMER);
    break;

    case UI_INPUT_P_CARD_UNDO:
    ProcessUW2FUndo();
    StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);

    break;

    case UI_INPUT_BLACK_PCARD_LEFT:
    if(m_PCardLeft == 2){
      m_PCardLeft +=2;
      StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
    }
    break;

    case UI_INPUT_BLACK_PCARD_RIGHT:
    if(m_PCardRight == 2){
      m_PCardRight +=2;
      StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
    }
    break;

    case UI_INPUT_BLACK_PCARD_LEFT_DECR:

    if(m_PCardLeft == 4){
      m_PCardLeft =2;
      StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
    }
    break;

    case UI_INPUT_BLACK_PCARD_RIGHT_DECR:
    if(m_PCardRight == 4){
      m_PCardRight =2;
      StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
    }
    break;

    case UI_INPUT_BUZZ:
    StateChanged(EVENT_TOGGLE_BUZZER);
    break;

    case UI_INPUT_RESTORE_UW2F_TIMER:

    m_UW2FTimer.RestorePreviousState();
    m_UW2FSeconds = m_UW2FTimer.GetIntermediateTime();
    StateChanged(EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);

    break;

    case UI_CYCLE_BRIGHTNESS:
    StateChanged(eventtype);
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

// New Rule after FIE Congress 26/11/2022
void FencingStateMachine::ProcessUW2F()
{

  if(m_PCardLeft == 2)
  {
    m_PCardRight+=2;
    m_PCardLeft+=2;
    return;
  }
    if(m_PCardLeft < 2)
    {
      m_PCardLeft++;
      if(m_PCardLeft == 2)
      {
        //m_ScoreRight++;
        incrementScoreAndCheckForMinuteBreak(false);
        StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
      }
    }

    if(m_PCardRight < 2)
    {
      m_PCardRight++;
      if(m_PCardRight == 2)
      {
        //m_ScoreLeft++;
        incrementScoreAndCheckForMinuteBreak(true);;
        StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
      }
    }

}

void FencingStateMachine::ProcessUW2FUndo()
{
  if(m_PCardLeft == 0){  // in this case also m_PCardRight = 0 -> noting to do
    return;
  }

  if(m_PCardLeft == 1){  // in this case also m_PCardRight = 1 -> just remove the cards, no score change needed
    m_PCardLeft = 0;
    m_PCardRight = 0;
    m_UW2FTimer.RestorePreviousState();
    m_UW2FSeconds = m_UW2FTimer.GetIntermediateTime();
    StateChanged(EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);
    return;
  }


  if((m_PCardLeft == 2) && (m_PCardRight == 2)){   //-> show yellow cards and change the score
    m_PCardLeft = 1;
    m_PCardRight = 1;
    m_ScoreRight--;
    m_ScoreLeft--;
    StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
    StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
    m_UW2FTimer.RestorePreviousState();
    m_UW2FSeconds = m_UW2FTimer.GetIntermediateTime();
    StateChanged(EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);
    return;
  }
  if((m_PCardLeft == 4) && (m_PCardRight == 4)){   //-> show yellow cards and change the score
    m_PCardLeft = 2;
    m_PCardRight = 2;

    m_UW2FTimer.RestorePreviousState();
    m_UW2FSeconds = m_UW2FTimer.GetIntermediateTime();
    StateChanged(EVENT_UW2F_TIMER | (m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);
    return;
  }
}

uint32_t FencingStateMachine::MakeTimerEvent()
{
  uint32_t temp = (m_Timer.GetMinutes()<<16 | m_Timer.GetSeconds()<<8  | m_Timer.GetHundredths());

  temp |= EVENT_TIMER;
  return temp;
}
void FencingStateMachine::ClearAllCards(bool bIncludePCards)
{
  m_YellowCardLeft =0;
  m_YellowCardRight =0;
  m_RedCardLeft =0;
  m_RedCardRight =0;
  m_BlackCardLeft =0;
  m_BlackCardRight =0;
  if(bIncludePCards){
    m_YellowPCardLeft =0;
    m_YellowPCardRight =0;
    m_RedPCardLeft =0;
    m_RedPCardRight =0;
    m_BlackPCardLeft =0;
    m_BlackPCardRight =0;
    m_PCardLeft = 0;
    m_PCardRight = 0;
    StateChanged(EVENT_P_CARD);
  }
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
          ClearAllCards(false);
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
  //while (!xSemaphoreTake(timerSemaphore_FSMPeriod, 1 / (portTICK_PERIOD_MS)) == pdTRUE); // check with 1 ms timeout
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
        if(m_GlobalIdle){
          StateChanged(EVENT_IDLE | EVENT_GO_OUT_OF_IDLE);
          m_GlobalIdle = false;
        }
        m_LastLightEventTime = millis();
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
            StateChanged(MakeTimerEvent());
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
    if(m_GlobalIdle){
      StateChanged(EVENT_IDLE | EVENT_GO_OUT_OF_IDLE);
      m_GlobalIdle = false;
    }
    m_LastLightEventTime = millis();
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
  else {
    if(!m_GlobalIdle){
      if(EPEE == m_MachineWeapon){
        if(m_LastLightEventTime + IDLE_TIME_MS < millis()){
          StateChanged(EVENT_IDLE | EVENT_GO_INTO_IDLE);
          m_GlobalIdle = true;
        }
      }
    }
    else{
      if(m_LastLightEventTime + IDLE_TIME_MS + TIME_TO_DEEP_SLEEP< millis()){
          m_GoToSleep = true;
        }
    }
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

#define emptystring ""
void FencingStateMachine::ProcessDisplayMessage (const EFP1Message &input)
{
  // I'm not sure this is a good idea, as Cyrano has no fiels for this

  m_UW2FTimer.Reset();
  StateChanged(EVENT_UW2F_TIMER);


  if(input[Start_time] != emptystring)
  {
  	//Do Something with input[Start_time];
  }

  if(input[StopWatch] != emptystring)
  {
  	//Do Something with input[StopWatch];
    uint8_t minutes = 3;
    uint8_t seconds = 0;
    sscanf(input[StopWatch].c_str(),"%d:%d",& minutes, & seconds);
    m_Timer.SetMinutes(minutes);
    m_Timer.SetSeconds(seconds);
    m_Timer.SetHundredths(0);

    StateChanged(MakeTimerEvent());
  }
  if(input[CompetitionType] != emptystring)
  {
  	//Do Something with input[CompetitionType];
    if("T" == input[CompetitionType])
    {
      m_nrOfRounds = 9;
    }
  }

  if(input[RoundNumber] != emptystring)
  {
  	//Do Something with input[RoundNumber];
    int temp;
    sscanf(input[RoundNumber].c_str(),"%d",& temp);
    if(temp == m_currentRound+1){
      SetNextTimerStateAndRoundAndNewTimeOnTimerZero();
      StateChanged(EVENT_TIMER_STATE);
      StateChanged(EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);
    }
    else{
      m_currentRound = temp;
      StateChanged(EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);
    }
  }
  if(input[Weapon] != emptystring)
  {
  	//Do Something with input[Weapon];
    m_TheSensor->Setweapon_detection_mode(MANUAL);
    if("F" == input[Weapon])
    {
      m_MachineWeapon = FOIL;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_FOIL);
      m_TheSensor->SetActualWeapon(FOIL);
    }
    if("E" == input[Weapon])
    {
      m_MachineWeapon = EPEE;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_EPEE);
      m_TheSensor->SetActualWeapon(EPEE);
    }
    if("S" == input[Weapon])
    {
      m_MachineWeapon = SABRE;
      StateChanged(EVENT_WEAPON | WEAPON_MASK_SABRE);
      m_TheSensor->SetActualWeapon(SABRE);
    }

  }

  if(input[Priority] != emptystring)
  {
  	//Do Something with input[Priority];
    if("L" == input[Priority])
    {
      m_Priority = PRIO_LEFT;
      StateChanged(EVENT_PRIO | 1);
    }
    if("R" == input[Priority])
    {
      m_Priority = PRIO_RIGHT;
      StateChanged(EVENT_PRIO | 2);
    }
    if("N" == input[Priority])
    {
      m_Priority = NO_PRIO;
      StateChanged(EVENT_PRIO);
    }
  }


  if(input[State] != emptystring)
  {
  	//Do Something with input[State];
  }


  if(input[RightFencerId] != emptystring)
  {
  	//Do Something with input[RightFencerId];
  }

  if(input[RightFencerName] != emptystring)
  {
  	//Do Something with input[RightFencerName];
  }

  if(input[RightFencerNation] != emptystring)
  {
  	//Do Something with input[RightFencerNation];
  }

  if(input[RightScore] != emptystring)
  {
  	//Do Something with input[RightScore];
    sscanf(input[RightScore].c_str(),"%d",&m_ScoreRight);
    StateChanged(EVENT_SCORE_RIGHT | m_ScoreRight);
  }

  if(input[RightStatus] != emptystring)
  {
  	//Do Something with input[RightStatus];
  }

  if(input[RightYCard] != emptystring)
  {
  	//Do Something with input[RightYCard];
    sscanf(input[RightYCard].c_str(),"%d",& m_YellowCardRight);
    StateChanged(m_YellowCardRight | EVENT_YELLOW_CARD_RIGHT );
  }

  if(input[RightRCard] != emptystring)
  {
  	//Do Something with input[RightRCard];
    sscanf(input[RightRCard].c_str(),"%d",& m_RedCardRight);
    StateChanged(m_RedCardRight | EVENT_RED_CARD_RIGHT );
  }


  if(input[RightMedicalIntervention] != emptystring)
  {
  	//Do Something with input[RightMedicalIntervention];
  }

  if(input[RightReserveIntroduction] != emptystring)
  {
  	//Do Something with input[RightReserveIntroduction];
  }

  if(input[RightPCards] != emptystring)
  {
  	//Do Something with input[RightPCards];
    sscanf(input[RightPCards].c_str(),"%d",& m_PCardRight);
    StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);

  }

  if(input[LeftFencerId] != emptystring)
  {
  	//Do Something with input[LeftFencerId];
  }

  if(input[LeftFencerName] != emptystring)
  {
  	//Do Something with input[LeftFencerName];
  }

  if(input[LeftFencerNation] != emptystring)
  {
  	//Do Something with input[LeftFencerNation];
  }

  if(input[LeftScore] != emptystring)
  {
  	//Do Something with input[LeftScore];
    sscanf(input[LeftScore].c_str(),"%d",& m_ScoreLeft);
    StateChanged(EVENT_SCORE_LEFT | m_ScoreLeft);
  }

  if(input[LeftStatus] != emptystring)
  {
  	//Do Something with input[LeftStatus];
  }

  if(input[LeftYCard] != emptystring)
  {
  	//Do Something with input[LeftYCard];
    sscanf(input[LeftYCard].c_str(),"%d",& m_YellowCardLeft);
    StateChanged(m_YellowCardLeft | EVENT_YELLOW_CARD_LEFT );

  }

  if(input[LeftRCard] != emptystring)
  {
  	//Do Something with input[LeftRCard];
    sscanf(input[LeftRCard].c_str(),"%d",& m_RedCardLeft);
    StateChanged(m_RedCardLeft | EVENT_RED_CARD_LEFT );
  }


  if(input[LeftMedicalIntervention] != emptystring)
  {
  	//Do Something with input[LeftMedicalIntervention];
  }

  if(input[LeftReserveIntroduction] != emptystring)
  {
  	//Do Something with input[LeftReserveIntroduction];
  }

  if(input[LeftPCards] != emptystring)
  {
  	//Do Something with input[LeftPCards];
    sscanf(input[LeftPCards].c_str(),"%d",& m_PCardLeft);
    StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
  }
}

uint32_t FencingStateMachine::get_max_score()
{
  switch(m_nrOfRounds)
    {
      case 1:
      return 5;
      break;

      case 2:
      return 10;
      break;

      case 3:
      return 15;
      break;

      case 9:
      return m_currentRound * 5;
      break;
    }
    return 5;
}

bool FencingStateMachine::incrementScoreAndCheckForMinuteBreak(bool bLeftFencer)
{
  // I should check what the current max score is
  if(bLeftFencer)
  {
    if(m_ScoreLeft < get_max_score())
      m_ScoreLeft++;
  }
  else
  {
    if( m_ScoreRight < get_max_score())
      m_ScoreRight++;
  }
  if((m_MachineWeapon == SABRE) && (1== m_currentRound))
  {
      if((8 == m_ScoreRight) || (8 == m_ScoreLeft))
      {
        StateChanged(EVENT_TIMER);
        SetNextTimerStateAndRoundAndNewTimeOnTimerZero();
        StateChanged(EVENT_ROUND | m_currentRound | m_nrOfRounds<<8);
        StateChanged(MakeTimerEvent());
        return true;
      }
  }
  return false;

}



void FencingStateMachine::PeriodicallyBroadcastFullState(class RepeaterSender *TheRepeater,long Period){
  if(millis() > m_TimeToBroadcastFullState)
  {
    m_TimeToBroadcastFullState = millis() + Period;
    TransmitFullStateToDisplay(TheRepeater);
  }
}
