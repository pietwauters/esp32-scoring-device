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
    pinMode(BUZZERPIN, OUTPUT);
    digitalWrite(BUZZERPIN, HIGH);
    m_pixels = new Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
    m_pixels->setBrightness(m_Brightness);
    m_Red = Adafruit_NeoPixel::Color(255, 0, 0, m_Brightness);
    m_Green = Adafruit_NeoPixel::Color(0, 255, 0, m_Brightness);
    m_White = Adafruit_NeoPixel::Color(255, 255, 255, m_Brightness);
    m_Orange = Adafruit_NeoPixel::Color(160, 60, 0, m_Brightness);
    m_Yellow = Adafruit_NeoPixel::Color(255, 210, 0, m_Brightness);
    m_Off = Adafruit_NeoPixel::Color(0, 0, 0, m_Brightness);
    queue = xQueueCreate( 60, sizeof( int ) );
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

void WS2812B_LedStrip::setWhiteLeft(bool Value)
{
    if(Value)
    {
        for(int i=0;i<64;i++)
        {
          if(Cross[i])
            m_pixels->setPixelColor(i, m_White); // Moderately bright green color.
          else
            m_pixels->setPixelColor(i, m_pixels->Color(0,0,0)); // Moderately bright green color.
        }

    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),0,64);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setWhiteRight(bool Value)
{
    if(Value)
    {
        for(int i=64;i<128;i++)
        {
          if(Cross[i-64])
            m_pixels->setPixelColor(i, m_White); // Moderately bright green color.
          else
            m_pixels->setPixelColor(i, m_pixels->Color(0,0,0)); // Moderately bright green color.
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
        m_pixels->fill(m_Orange,40-16,24);
    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),40-16,24);
    }
    //m_pixels->show();   // Send the updated pixel colors to the hardware.
}

void WS2812B_LedStrip::setOrangeRight(bool Value)
{
    if(Value)
    {
        m_pixels->fill(m_Orange,104-16,24);

    }
    else
    {
        m_pixels->fill(m_pixels->Color(0, 0, 0),104-16,24);
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

void WS2812B_LedStrip::setBuzz(bool Value)
{
    if(m_Loudness)
    {
        if(Value)
        {
            digitalWrite(BUZZERPIN, LOW);
        }
        else
        {
            digitalWrite(BUZZERPIN, HIGH);
        }
    }

}
