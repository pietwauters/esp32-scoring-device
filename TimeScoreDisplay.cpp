//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "TimeScoreDisplay.h"
#include "hardwaredefinition.h"

#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES 4




static const int spiClk = 1000000; // 1 MHz

//uninitalised pointers to SPI objects
SPIClass hspi(HSPI);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, hspi, CS_PIN, MAX_DEVICES);


uint8_t numbers[][9]= {{5,62,81,73,69,62,0,0,0},
{3,66,127,64,0,0,0,0,0},
{5,113,73,73,73,70,0,0,0},
{5,65,73,73,73,54,0,0,0},
{5,15,8,8,8,127,0,0,0},
{5,79,73,73,73,49,0,0,0},
{5,62,73,73,73,48,0,0,0},
{5,3,1,1,1,127,0,0,0},
{5,54,73,73,73,54,0,0,0},
{5,6,73,73,73,62,0,0,0},  // 9
{2,108,108,0,0,0,0,0,0},  // :
{4,8,8,8,8,0,0,0,0},      // -
{2,96,96,0,0,0,0,0,0},    // .
{5,32,16,8,4,2,0,0,0},    // /
{5,99,20,8,20,99,0,0,0},  // x
{5,124,18,17,18,124,0,0,0}, // A (offset 15)
{5,127,73,73,73,54,0,0,0},
{5,62,65,65,65,34,0,0,0},
{5,127,65,65,65,62,0,0,0},
{5,127,73,73,73,65,0,0,0},
{5,127,9,9,9,1,0,0,0},
{5,62,65,65,81,115,0,0,0},
{5,127,8,8,8,127,0,0,0},
{5,0,65,127,65,0,0,0,0},
{5,32,64,65,63,1,0,0,0},
{5,127,8,20,34,65,0,0,0},
{5,127,64,64,64,64,0,0,0},
{5,127,2,28,2,127,0,0,0},
{5,127,4,8,16,127,0,0,0},
{5,62,65,65,65,62,0,0,0},
{5,127,9,9,9,6,0,0,0},
{5,62,65,81,33,94,0,0,0},
{5,127,9,25,41,70,0,0,0},
{5,38,73,73,73,50,0,0,0},
{5,3,1,127,1,3,0,0,0},
{5,63,64,64,64,63,0,0,0},
{5,31,32,64,32,31,0,0,0},
{5,63,64,56,64,63,0,0,0},
{5,99,20,8,20,99,0,0,0},
{5,3,4,120,4,3,0,0,0},
{5,97,89,73,77,67,0,0,0} // Z
};


#define TIME_SCORE_PERIOD_INTERVAL_MS 2000


void TimeScoreDisplay::SetBrightness(int value)
{
  m_Brightness = value; mx.control(MD_MAX72XX::INTENSITY, value);
}

TimeScoreDisplay::TimeScoreDisplay()
{
    //ctor
}

TimeScoreDisplay::~TimeScoreDisplay()
{
    //dtor
}

void TimeScoreDisplay::begin()
{
    gpio_hold_dis((gpio_num_t)HSPI_SS);
    hspi.begin();
    pinMode(HSPI_SS, OUTPUT); //HSPI SS
    //Switch the power of all LED modules on
    pinMode(PowerPin, OUTPUT);
    digitalWrite(PowerPin,HIGH);

    mx.begin();
    mx.clear();
    queue = xQueueCreate( 60, sizeof( int ) );
    SetBrightness(TEXT_BRIGHTNESS_NORMAL);
    Preferences networkpreferences;
    networkpreferences.begin("credentials", false);
    PisteId = networkpreferences.getInt("pisteNr", 500);
    networkpreferences.end();
}

void TimeScoreDisplay::SetChar(uint8_t MostLeftPosition, uint8_t character)
{
  mx.update(MD_MAX72XX::OFF);
  uint8_t startsegment = MostLeftPosition >> 3;
  uint8_t start = (startsegment +1) * COL_SIZE -1 - MostLeftPosition%COL_SIZE;
  uint8_t boundary = startsegment << 3;

  {
    for(int i=0; i < numbers[character][0]; i++)
    {
      if(start-i < boundary)
        start += COL_SIZE*2;
      mx.setColumn(start-i,numbers[character][i+1]);
    }
  }
  mx.update();
}
void TimeScoreDisplay::ClearColumn(uint8_t MostLeftPosition)
{
  mx.update(MD_MAX72XX::OFF);
  uint8_t startsegment = MostLeftPosition/COL_SIZE;
  uint8_t start = (startsegment +1) * COL_SIZE -1 - MostLeftPosition%COL_SIZE;
  mx.setColumn(start, 0x00);
  mx.update();

}

void TimeScoreDisplay::DisplayScore(uint8_t scoreLeft, uint8_t scoreRight)
{
  mx.clear();
  uint8_t digit0 = scoreLeft /10;
  uint8_t digit1 = scoreLeft - digit0 * 10;
  uint8_t digit2 = scoreRight /10;
  uint8_t digit3 = scoreRight - digit2 * 10;
  uint8_t w = numbers[digit0][0];
  uint8_t diff = 5 - w;

  SetChar(0 + diff/2,digit0);
  w = numbers[digit1][0];
  diff = 5 - w;
  SetChar(6 + diff/2,digit1);
  SetChar(14,11);

  w = numbers[digit2][0];
  diff = 5 - w;
  SetChar(21 + diff/2,digit2);

  w = numbers[digit3][0];
  diff = 5 - w;
  SetChar(32-w - diff/2,digit3);

}

void TimeScoreDisplay::DisplayTime(uint8_t minutes, uint8_t seconds, uint8_t hundreths,bool TenthsOnly)
{
  mx.clear();
  uint8_t digit0 = minutes;
  uint8_t digit1 = seconds/10;
  uint8_t digit2 = seconds - digit1*10;
  bool bDisplayDigit2 = true;

  if((minutes == 0) && (seconds < 10))
  {
    digit0 = seconds;
    digit1 = hundreths/10;
    digit2 = hundreths - digit1*10;
    SetChar(13,12);    // .
    if(TIMER_RUNNING == m_TimerStatus)
    {
      bDisplayDigit2 = false;
    }
  }
  else
  {
    SetChar(13,10);    // ::
  }

  uint8_t w = numbers[digit0][0];
  uint8_t diff = 5 - w;
  SetChar(6 + diff/2,digit0);
  w = numbers[digit1][0];
  diff = 5 - w;
  SetChar(18 - diff/2,digit1);
  if(bDisplayDigit2)
    SetChar(18+numbers[digit1][0]+1 - diff/2,digit2);

}

void TimeScoreDisplay::DisplayMatchCount(uint8_t match, uint8_t maxmatch)
{
  mx.clear();
  uint8_t digit0 = match;
  if(digit0 > 9)
  {
      digit0 = 14;
  }
  uint8_t digit1 = maxmatch;

  uint8_t w = numbers[digit0][0];
  uint8_t diff = 5 - w;

  SetChar(13,13);    // /

  SetChar(13-3 - w - diff/2,digit0);
  w = numbers[digit1][0];
  diff = 5 - w;
  SetChar(21 + diff/2,digit1);

}

void TimeScoreDisplay::update (FencingStateMachine *subject, uint32_t eventtype)
{
  xQueueSend(queue, &eventtype, portMAX_DELAY);
}

void TimeScoreDisplay::update (RepeaterReceiver *subject, uint32_t eventtype)
{
  xQueueSend(queue, &eventtype, portMAX_DELAY);
}

#define MASK_RED_OR_GREEN 0x84

void TimeScoreDisplay::ProcessEvents ()
{
  if(queue == NULL)
    return;
  if(uxQueueMessagesWaiting(queue) == 0 )
    return;

  xQueueReceive(queue, &m_LastEvent, portMAX_DELAY);
  uint32_t event_data = m_LastEvent & SUB_TYPE_MASK;
  uint32_t maineventtype = m_LastEvent & MAIN_TYPE_MASK ;
  uint32_t tempevent = m_LastEvent;


  char chrono[16];
  char strRound[8];
  int newseconds;


  switch(maineventtype)
  {
    /*case EVENT_LIGHTS:
      if(event_data && MASK_RED_OR_GREEN)
        ShowScoreForGivenDuration(5000);
    break;*/
    case EVENT_IDLE:
    if(event_data == EVENT_GO_INTO_IDLE){
      m_Idle = true;
      mx.control(MD_MAX72XX::SHUTDOWN, MD_MAX72XX::ON);
      SetPower(false);
    }
    else {
      SetPower(true); // this will also shut_down the WS2812B panels
      m_Idle = false;
      mx.begin();
      mx.clear();
      mx.control(MD_MAX72XX::SHUTDOWN, MD_MAX72XX::OFF);

    }



    break;
    case EVENT_UI_INPUT:
    switch (event_data)
    {
      case UI_CYCLE_BRIGHTNESS:
      switch(m_Brightness)
      {
        case TEXT_BRIGHTNESS_LOW:
        SetBrightness(TEXT_BRIGHTNESS_NORMAL);
        break;
        case TEXT_BRIGHTNESS_NORMAL:
        SetBrightness(TEXT_BRIGHTNESS_HIGH);
        break;
        case TEXT_BRIGHTNESS_HIGH:
        SetBrightness(TEXT_BRIGHTNESS_ULTRAHIGH);
        break;
        case TEXT_BRIGHTNESS_ULTRAHIGH:
        SetBrightness(TEXT_BRIGHTNESS_LOW);
        break;

        default:
        SetBrightness(TEXT_BRIGHTNESS_NORMAL);
      }
      break;
    }

    break;

    case EVENT_ROUND:
      m_round =  event_data & DATA_BYTE0_MASK;
      m_maxround = (event_data & DATA_BYTE1_MASK) >>8;
      NextTimeToSwitchBetweenScoreAndTime = millis() + 2500;
      DisplayMatchCount(m_round,m_maxround);
    break;

    case EVENT_SCORE_LEFT:
      m_scoreLeft =  event_data;
      NextTimeToSwitchBetweenScoreAndTime = millis() + 2500;
      ShowScore();
    break;

    case EVENT_SCORE_RIGHT:
      m_scoreRight =  event_data;
      NextTimeToSwitchBetweenScoreAndTime = millis() + 2500;
      ShowScore();
    break;

    case EVENT_TIMER_STATE:

      if(tempevent & DATA_24BIT_MASK)
      {

        //Message2.SetTimerStatus('R');
        m_TimerStatus = TIMER_RUNNING;
        ShowTime();   // actually I don't want this in the eventhandler. It has to be as short as possible

      }
      else
      {

        //Message2.SetTimerStatus('N');
        m_TimerStatus = TIMER_STOPPED;


        if(3 != m_objectshown)
        {
          NextTimeToSwitchBetweenScoreAndTime = millis() + 5000;
          ShowScore();  // actually I don't want this in the eventhandler. It has to be as short as possible
        }

      }
    break;
    case EVENT_TIMER:
    newseconds = event_data & (DATA_BYTE1_MASK |DATA_BYTE2_MASK);
    mix_t TimeInfo;
    TimeInfo.theDWord = tempevent & DATA_24BIT_MASK;
    m_seconds = TimeInfo.theBytes[1];
    m_minutes = TimeInfo.theBytes[2];
    m_hundredths = TimeInfo.theBytes[0];
    SetTime(m_minutes,m_seconds,m_hundredths);
    ShowTime();
    break;

    case EVENT_WEAPON:
    NextTimeToSwitchBetweenScoreAndTime = millis() + 4500;
    // below is needed because when I switch to foil with no weapons connected,
    // there will be a double hit, resulting in immediately showing the score
    m_objectshown = 3;
    switch(event_data)
    {
      case WEAPON_MASK_EPEE:
      DisplayWeapon(EPEE);
      break;

      case WEAPON_MASK_FOIL:
      DisplayWeapon(FOIL);

      break;

      case WEAPON_MASK_SABRE:
      DisplayWeapon(SABRE);
      break;

      case WEAPON_MASK_UNKNOWN:
      DisplayWeapon(UNKNOWN);
      break;

    }


    break;

  }

}

void TimeScoreDisplay::ShowScoreForGivenDuration(uint32_t duration)
{
  NextTimeToSwitchBetweenScoreAndTime = millis() + duration;
  ShowScore();
  m_objectshown = 2;
}

void TimeScoreDisplay::AlternateScoreAndTimeWhenNotFighting()
{

  if(TIMER_RUNNING == m_TimerStatus)
    return;
  if(millis() > NextTimeToSwitchBetweenScoreAndTime)
  {
    NextTimeToSwitchBetweenScoreAndTime = millis() + 2500;
    if(m_ScoreIsShown)
    {
      m_ScoreIsShown = false;
      ShowTime();
    }
    else
    {
      m_ScoreIsShown = true;
      ShowScore();
    }
  }
}

void TimeScoreDisplay::CycleScoreMatchAndTimeWhenNotFighting()
{

  if(TIMER_RUNNING == m_TimerStatus)
  {
    if((m_minutes == 0) && (m_seconds < 9))
      return;
    if(millis() > NextTimeToTogglecolon)
    {
      NextTimeToTogglecolon = millis() + 250;
      if(m_separatorshown)
      {
        m_separatorshown = false;
        ClearColumn(13);
        ClearColumn(14);
      }
      else
      {
        SetChar(13,10);
        m_separatorshown = true;
      }

    }


    return;
  }

  if(millis() > NextTimeToSwitchBetweenScoreAndTime)
  {

    switch (m_objectshown)
    {
      case 0:
      ShowTime();
      m_objectshown = 1;
      NextTimeToSwitchBetweenScoreAndTime = millis() + TIME_SCORE_PERIOD_INTERVAL_MS;
      break;

      case 1:
      ShowScore();
      m_objectshown = 2;
      NextTimeToSwitchBetweenScoreAndTime = millis() + TIME_SCORE_PERIOD_INTERVAL_MS;
      break;

      case 2:
      DisplayMatchCount(m_round, m_maxround);
      m_objectshown = 0;
      NextTimeToSwitchBetweenScoreAndTime = millis() + TIME_SCORE_PERIOD_INTERVAL_MS / 3;
      break;

      case 3:
      ShowTime();
      m_objectshown = 1;
      NextTimeToSwitchBetweenScoreAndTime = millis() + TIME_SCORE_PERIOD_INTERVAL_MS;
      break;

    }

  }
}

void TimeScoreDisplay::DisplayPisteId()
{
  mx.clear();
  char text[6];
  sprintf(text,"P-%03d",PisteId);
  uint8_t digit0 = text[0] - 'A' + 15;
  uint8_t digit1 = 11;
  uint8_t digit2 = text[2] - '0' ;
  uint8_t digit3 = text[3] - '0' ;
  uint8_t digit4 = text[4] - '0' ;
  uint8_t startpos = 2;
  uint8_t w = numbers[digit0][0] + 1 + startpos;

  SetChar(startpos,digit0);
  SetChar(w,digit1);
  w = w + numbers[digit1][0] + 1;
  SetChar(w,digit2);
  w = w + numbers[digit2][0] + 1;
  SetChar(w,digit3);
  w = w + numbers[digit3][0] + 1;
  SetChar(w,digit4);

}

void TimeScoreDisplay::DisplayWeapon(weapon_t weapon)
{
  mx.clear();
  uint8_t digit0 = 'E' - 'A' + 15;
  uint8_t digit1 = 'P' - 'A' + 15;
  uint8_t digit2 = 'E' - 'A' + 15;
  uint8_t digit3 = 'E' - 'A' + 15;
  uint8_t digit4 = 'E' - 'A' + 15;

  uint8_t startpos = 4;
  switch(weapon)
  {
    case FOIL:
      digit0 = 'F' - 'A' + 15;
      digit1 = 'O' - 'A' + 15;
      digit2 = 'I' - 'A' + 15;
      digit3 = 'L' - 'A' + 15;
    break;

    case SABRE:
      digit0 = 'S' - 'A' + 15;
      digit1 = 'A' - 'A' + 15;
      digit2 = 'B' - 'A' + 15;
      digit3 = 'R' - 'A' + 15;
      startpos = 1;

    break;

    case UNKNOWN:
      digit0 = 'A' - 'A' + 15;
      digit1 = 'U' - 'A' + 15;
      digit2 = 'T' - 'A' + 15;
      digit3 = 'O' - 'A' + 15;

    break;

  }
  uint8_t w = numbers[digit0][0] + 1 + startpos;

  SetChar(startpos,digit0);
  SetChar(w,digit1);
  w = w + numbers[digit1][0] + 1;
  SetChar(w,digit2);
  w = w + numbers[digit2][0] + 1;
  SetChar(w,digit3);
  if(SABRE == weapon)
  {
    w = w + numbers[digit3][0] + 1;
    SetChar(w,digit4);
  }


}
