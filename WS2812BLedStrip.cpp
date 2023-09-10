//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "WS2812BLedStrip.h"


char Cross[] = {1,1,0,0,0,0,1,1,
                1,1,1,0,0,1,1,1,
                0,1,1,1,1,1,1,0,
                0,0,1,1,1,1,0,0,
                0,0,1,1,1,1,0,0,
                0,1,1,1,1,1,1,0,
                1,1,1,0,0,1,1,1,
                1,1,0,0,0,0,1,1};

WS2812B_LedStrip::WS2812B_LedStrip()
{
    //ctor
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN, LOW);   // not sure if this should be high or low. Low for a long time will reset the leds causing the content
    pinMode(BUZZERPIN, OUTPUT);
    digitalWrite(BUZZERPIN, RELATIVE_LOW);
    /*m_pixels = new Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
    m_pixels->fill(m_pixels->Color(0, 0, 0),0,NUMPIXELS);
    SetBrightness(BRIGHTNESS_NORMAL);*/

    queue = xQueueCreate( 60, sizeof( int ) );
}
void WS2812B_LedStrip::begin()
{
  m_pixels = new Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
  m_pixels->begin();
  m_pixels->clear();
  m_pixels->show();
  //m_pixels->fill(m_pixels->Color(0, 0, 0),0,NUMPIXELS);
  SetBrightness(BRIGHTNESS_NORMAL);
  //m_pixels->fill(m_pixels->Color(0, 0, 0),0,NUMPIXELS);
  m_pixels->show();
}

void WS2812B_LedStrip::SetBrightness(uint8_t val)
{
  m_Brightness = val;
  m_pixels->setBrightness(m_Brightness);
  m_Red = Adafruit_NeoPixel::Color(255, 0, 0, m_Brightness);
  m_Green = Adafruit_NeoPixel::Color(0, 255, 0, m_Brightness);
  m_White = Adafruit_NeoPixel::Color(200, 200, 200, m_Brightness);
  m_Orange = Adafruit_NeoPixel::Color(160, 60, 0, m_Brightness);
  m_Yellow = Adafruit_NeoPixel::Color(204, 168, 0, m_Brightness);
  m_Blue = Adafruit_NeoPixel::Color(0, 0, 255, m_Brightness);
  m_Off = Adafruit_NeoPixel::Color(0, 0, 0, m_Brightness);
}


WS2812B_LedStrip::~WS2812B_LedStrip()
{
    //dtor
    delete m_pixels;
}

void WS2812B_LedStrip::setRed(bool Value)
{
    if(Value)
    {
        //m_pixels->fill(m_pixels->Color(0, 120, 0),0,NUMPIXELS/3);
        //m_pixels->fill(m_Red,0,NUMPIXELS/2);
        for(int i=0;i<64;i++)
        {

            m_pixels->setPixelColor(i, m_Red); // Moderately bright green color.
        }
    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),0,64);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setRedPrio(bool Value)
{
    if(Value)
    {
         m_pixels->fill(m_Red,0,8);
    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),0,8);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setGreen(bool Value)
{
    if(Value)
    {
        m_pixels->fill(m_Green,64,64);

    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),64,64);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setGreenPrio(bool Value)
{
    if(Value)
    {
        m_pixels->fill(m_Green,64,8);

    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),64,8);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setWhiteLeft(bool Value, bool inverse)
{
    uint32_t theFillColor = m_White;
    uint32_t theNotFillColor = m_Off;

    if(Value)
    {
      if(inverse)
      {
        theFillColor = m_Off;
        theNotFillColor = m_Blue;
      }

        for(int i=0;i<64;i++)
        {
          if(Cross[i])
            m_pixels->setPixelColor(i, theFillColor); // Moderately bright green color.
          else
            m_pixels->setPixelColor(i, theNotFillColor); // Moderately bright green color.
        }

    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),0,64);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setWhiteRight(bool Value, bool inverse)
{
  uint32_t theFillColor = m_White;
  uint32_t theNotFillColor = m_Off;

    if(Value)
    {
      if(inverse)
      {
        theFillColor = m_Off;
        theNotFillColor = m_Blue;
      }
        for(int i=64;i<128;i++)
        {
          if(Cross[i-64])
            m_pixels->setPixelColor(i, theFillColor); // Moderately bright green color.
          else
            m_pixels->setPixelColor(i, theNotFillColor); // Moderately bright green color.
        }

    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),64,64);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setOrangeLeft(bool Value)
{
    if(Value)
    {
        m_pixels->fill(m_Orange,40-16,16);
    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),40-16,16);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setOrangeRight(bool Value)
{
    if(Value)
    {
        m_pixels->fill(m_Orange,104-16,16);

    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),104-16,16);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setYellowCardRight(bool Value)
{
  uint32_t theFillColor = m_Off;
    if(Value)
    {
        theFillColor = m_Yellow;
    }
    m_pixels->fill(theFillColor,70+5*8,2);
    m_pixels->fill(theFillColor,70+6*8,2);
    m_pixels->fill(theFillColor,70+7*8,2);
}

void WS2812B_LedStrip::setYellowCardLeft(bool Value)
{
    uint32_t theFillColor = m_Off;
    if(Value)
    {
        theFillColor = m_Yellow;
    }
    m_pixels->fill(theFillColor,0+5*8,2);
    m_pixels->fill(theFillColor,0+6*8,2);
    m_pixels->fill(theFillColor,0+7*8,2);

    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setRedCardRight(bool Value)
{
    uint32_t theFillColor = m_Off;
    if(Value)
    {
        theFillColor = m_Red;
    }
    m_pixels->fill(theFillColor,68+5*8,2);
    m_pixels->fill(theFillColor,68+6*8,2);
    m_pixels->fill(theFillColor,68+7*8,2);

}

void WS2812B_LedStrip::setRedCardLeft(bool Value)
{
    uint32_t theFillColor = m_Off;
    if(Value)
    {
        theFillColor = m_Red;
    }
    m_pixels->fill(theFillColor,2+5*8,2);
    m_pixels->fill(theFillColor,2+6*8,2);
    m_pixels->fill(theFillColor,2+7*8,2);
}

void WS2812B_LedStrip:: update (FencingStateMachine *subject, uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  if(EVENT_LIGHTS == maineventtype)
  {
    //SetLedStatus((unsigned char)event_data);
    xQueueSend(queue, &eventtype, portMAX_DELAY);
  }
  switch(maineventtype)
  {
    case EVENT_UI_INPUT:
    switch (event_data)
    {
      case UI_CYCLE_BRIGHTNESS:
      switch(m_Brightness)
      {
        case BRIGHTNESS_LOW:
        SetBrightness(BRIGHTNESS_NORMAL);
        break;
        case BRIGHTNESS_NORMAL:
        SetBrightness(BRIGHTNESS_HIGH);
        break;
        case BRIGHTNESS_HIGH:
        SetBrightness(BRIGHTNESS_ULTRAHIGH);
        break;
        case BRIGHTNESS_ULTRAHIGH:
        SetBrightness(BRIGHTNESS_LOW);
        break;

        default:
        SetBrightness(BRIGHTNESS_NORMAL);
      }
      break;
    }

    break;

    case EVENT_PRIO:
    StartPrioAnimation(event_data);
    break;

    case EVENT_YELLOW_CARD_LEFT:
    if(event_data)
    {
      m_YellowCardLeft = true;

    }
    else
    {
      m_YellowCardLeft = false;
    }
    setYellowCardLeft(m_YellowCardLeft);
    SetLedStatus(0xff);
    break;

    case EVENT_YELLOW_CARD_RIGHT:
    if(event_data)
    {
      m_YellowCardRight = true;
    }
    else
    {
      m_YellowCardRight = false;
    }
    setYellowCardRight(m_YellowCardRight);
    SetLedStatus(0xff);
    break;

    case EVENT_RED_CARD_LEFT:

    if(event_data)
    {
      m_RedCardLeft = true;
    }
    else
    {
      m_RedCardLeft = false;
    }
    setRedCardLeft(m_RedCardLeft);
    SetLedStatus(0xff);
    break;

    case EVENT_RED_CARD_RIGHT:

    if(event_data)
    {
      m_RedCardRight = true;
    }
    else
    {
      m_RedCardRight = false;
    }
    setRedCardRight(m_RedCardRight);
    SetLedStatus(0xff);
    break;

    case EVENT_TOGGLE_BUZZER:
        m_Loudness = !m_Loudness;
    break;

    case EVENT_UW2F_TIMER:
//m_UW2FSeconds/60)<<16 | (m_UW2FSeconds%60)<<8);

    m_UW2Ftens = (event_data >> 8)/10 ;

    mix_t TimeInfo;
    TimeInfo.theDWord = event_data & DATA_24BIT_MASK;
    //m_seconds = TimeInfo.theBytes[1];
    //m_minutes = TimeInfo.theBytes[2];
    //m_hundredths = TimeInfo.theBytes[0];
    m_UW2Ftens = (TimeInfo.theBytes[2] *60 + TimeInfo.theBytes[1]) / 10;
    setUWFTimeLeft(m_UW2Ftens);
    setUWFTimeRight(m_UW2Ftens);
    SetLedStatus(0xff);
    break;

    case EVENT_TIMER:
      if(!event_data)
        StartWarning(11);
    break;

    case EVENT_P_CARD:
        //StateChanged(EVENT_P_CARD |  m_PCardLeft | m_PCardRight << 8);
        mix_t PCardInfo;
        PCardInfo.theDWord = event_data;
        switch (PCardInfo.theBytes[0])
        {
          case 0:
          m_YellowPCardLeft = false;
          m_RedPCardLeft = 0;
          break;

          case 1:
          m_YellowPCardLeft = true;
          m_RedPCardLeft = 0;
          break;

          case 2:
          m_YellowPCardLeft = true;
          m_RedPCardLeft = 1;
          break;

          case 3:
          m_YellowPCardLeft = true;
          m_RedPCardLeft = 2;
          break;

        }

        switch (PCardInfo.theBytes[1])
        {
          case 0:
          m_YellowPCardRight = false;
          m_RedPCardRight = 0;
          break;

          case 1:
          m_YellowPCardRight = true;
          m_RedPCardRight = 0;
          break;

          case 2:
          m_YellowPCardRight = true;
          m_RedPCardRight = 1;
          break;

          case 3:
          m_YellowPCardRight = true;
          m_RedPCardRight = 2;
          break;

        }


        SetLedStatus(0xff);
    break;

  }

}

void WS2812B_LedStrip::ProcessEvents ()
{
  if(queue == NULL)
    return;
  if(uxQueueMessagesWaiting(queue) == 0 )
    return;

  xQueueReceive(queue, &m_LastEvent, portMAX_DELAY);
  uint32_t event_data = m_LastEvent & SUB_TYPE_MASK;
  SetLedStatus((unsigned char)event_data);
}

void WS2812B_LedStrip::ProcessEventsBlocking ()
{
  if(xQueueReceive(queue, &m_LastEvent, 4 / portTICK_PERIOD_MS)== pdPASS)
  {
    uint32_t event_data = m_LastEvent & SUB_TYPE_MASK;
    SetLedStatus((unsigned char)event_data);
  }

}

void WS2812B_LedStrip::SetLedStatus(unsigned char val)
{
    if(val != 0xff)
    {
      if(m_LedStatus == val)
        return;
      m_LedStatus = val;
    }
    bool ColoredOn = m_LedStatus & MASK_RED;
    setRed(ColoredOn);
    if(!ColoredOn)
    {
      setWhiteLeft(m_LedStatus & MASK_WHITE_L);
      if(!(m_LedStatus & MASK_WHITE_L) )             // This is needed because I'm re-using the "white part" to show orange
      {
          setOrangeLeft(m_LedStatus & MASK_ORANGE_L);
          setRedPrio(m_PrioLeft);
          setYellowCardLeft(m_YellowCardLeft);
          setRedCardLeft(m_RedCardLeft);
          setUWFTimeLeft(m_UW2Ftens);
          setYellowPCardLeft(m_YellowPCardLeft);
          setRedPCardLeft(m_RedPCardLeft);
      }
    }

    ColoredOn = m_LedStatus & MASK_GREEN;
    setGreen(ColoredOn);
    if(!ColoredOn)
    {
      setWhiteRight(m_LedStatus & MASK_WHITE_R);
      if(!(m_LedStatus & MASK_WHITE_R) )
      {
        setOrangeRight(m_LedStatus & MASK_ORANGE_R);
        setGreenPrio(m_PrioRight);
        setYellowCardRight(m_YellowCardRight);
        setRedCardRight(m_RedCardRight);
        setUWFTimeRight(m_UW2Ftens);
        setYellowPCardRight(m_YellowPCardRight);
        setRedPCardRight(m_RedPCardRight);
      }
    }
    setBuzz(m_LedStatus & MASK_BUZZ);

    m_pixels->show();

}

void WS2812B_LedStrip::ClearAll()
{
   setRed(false);
   setWhiteLeft(false);
   setOrangeLeft(false);
   setOrangeRight(false);
   setWhiteRight(false);
   setGreen(false);
   setBuzz(false);
   myShow();
}

void WS2812B_LedStrip::AnimatePrio()
{
  if(!m_Animating)
    return;
  if(millis() < m_NextTimeToTogglePrioLights)
    return;

  m_NextTimeToTogglePrioLights = millis() + 60 + m_counter * 15;
  if(m_counter & 1)
  {
    setGreenPrio(false);
    setRedPrio(true);
  }
  else
  {
    setGreenPrio(true);
    setRedPrio(false);
  }
  m_pixels->show();
  m_counter--;
  if(m_counter < m_targetprio)
  {
    m_Animating = false;

  }


}

void WS2812B_LedStrip::StartPrioAnimation(uint8_t prio)
{
  switch(prio)
  {
    case 0:
      setGreenPrio(false);
      setRedPrio(false);
      m_pixels->show();
      m_PrioLeft = false;
      m_PrioRight = false;
      return;
    break;
    case 2:WS2812B_LedStrip::
      m_PrioLeft = false;
      m_PrioRight = true;
    break;

    case 1:
      m_PrioLeft = true;
      m_PrioRight = false;
    break;

  }
  m_targetprio = prio;

  m_counter = 17 + prio;
  m_NextTimeToTogglePrioLights = millis() + 100 + m_counter * 15;
  m_Animating = true;
}
#define WARNING_TIME_ON 150
#define WARNING_TIME_Off 40

void WS2812B_LedStrip::AnimateWarning()
{
  if(!m_WarningOngoing)
    return;
  if(millis() < m_NextTimeToToggleBuzzer)
    return;

  if(m_warningcounter & 1)
  {
    m_NextTimeToToggleBuzzer = millis() + WARNING_TIME_Off;
    setBuzz(false);
  }
  else
  {
    m_NextTimeToToggleBuzzer = millis() + WARNING_TIME_ON;
    setBuzz(true);
  }
  m_warningcounter--;
  if(!m_warningcounter)
  {
    setBuzz(false);
    m_WarningOngoing = false;
  }

}
void WS2812B_LedStrip::StartWarning(uint8_t nr_beeps)
{
  m_warningcounter = (nr_beeps * 2) + 1 ;
  m_NextTimeToToggleBuzzer = millis() + WARNING_TIME_ON;
  m_WarningOngoing = true;
}


void WS2812B_LedStrip::setBuzz(bool Value)
{
    if(m_Loudness)
    {
        if(Value)
        {
            digitalWrite(BUZZERPIN, RELATIVE_HIGH);
        }
        else
        {
            digitalWrite(BUZZERPIN, RELATIVE_LOW);
        }
    }

}

void WS2812B_LedStrip::setUWFTime(uint8_t tens, uint8_t bottom)
{
  if(tens > 8)
    tens = 8;
  if(tens == 0)
  {
    for(int i=0;i<8;i++)
    {
      m_pixels->setPixelColor(bottom - 8*i,m_Off);
    }
  }
  else
  {
    for(int i=0;i<tens;i++)
    {
      m_pixels->setPixelColor(bottom - 8*i,m_Blue);
    }
    for(int i=5;i<tens;i++)
    {
      {
        m_pixels->setPixelColor(bottom -i*8,m_Red);
      }
    }
  }
}

void WS2812B_LedStrip::setUWFTimeLeft(uint8_t tens)
{
  setUWFTime(tens,63);
}

void WS2812B_LedStrip::setUWFTimeRight(uint8_t tens)
{
  setUWFTime(tens,120);
}

void WS2812B_LedStrip::setYellowPCardRight(bool Value)
{
  uint32_t theFillColor = m_Off;
    if(Value)
    {
        theFillColor = m_Yellow;
    }
    m_pixels->setPixelColor(121,theFillColor);
    m_pixels->setPixelColor(121 - 8,theFillColor);

}

void WS2812B_LedStrip::setYellowPCardLeft(bool Value)
{
  uint32_t theFillColor = m_Off;
    if(Value)
    {
        theFillColor = m_Yellow;
    }
    m_pixels->setPixelColor(62,theFillColor);
    m_pixels->setPixelColor(62 - 8,theFillColor);

}

void WS2812B_LedStrip::setRedPCardRight(uint8_t nr)
{
  uint32_t theFillColor1 = m_Off;
  uint32_t theFillColor2 = m_Off;

    if(nr == 2)
    {
        //theFillColor1 = m_Red;
        //theFillColor2 = m_Red;
        setWhiteRight(true,true);
        return;
    }
    if(nr == 1)
    {
        theFillColor1 = m_Red;
    }

// Red2
    m_pixels->setPixelColor(122 + 1,theFillColor2);
    m_pixels->setPixelColor(122 + 1 - 8,theFillColor2);
// Red1
    m_pixels->setPixelColor(122 ,theFillColor1);
    m_pixels->setPixelColor(122 - 8,theFillColor1);

}

void WS2812B_LedStrip::setRedPCardLeft(uint8_t nr)
{
  uint32_t theFillColor1 = m_Off;
  uint32_t theFillColor2 = m_Off;

    if(nr == 2)
    {
        //theFillColor1 = m_Red;
        //theFillColor2 = m_Red;
        setWhiteLeft(true,true);
        return;
    }
    if(nr == 1)
    {
        theFillColor1 = m_Red;
    }
    m_pixels->setPixelColor(61 ,theFillColor1);
    m_pixels->setPixelColor(61 - 8,theFillColor1);
    m_pixels->setPixelColor(61 - 1,theFillColor2);
    m_pixels->setPixelColor(61 - 1  - 8,theFillColor2);

}
