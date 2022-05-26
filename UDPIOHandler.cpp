//Copyright (c) Piet Wauters 2022 <piet.wauters@gmail.com>
#include "UDPIOHandler.h"

#include <WiFi.h>
#include "AsyncUDP.h"

AsyncUDP Commandudp;

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "Linksys02978";
//char pass[] = "r2fz6bpzbs";
//const char * espHostName = "Scoring Machine ";
/*
WidgetLED ledRed(V12);
WidgetLED ledWhiteL(V13);
WidgetLED ledOrangeL(V14);
WidgetLED ledOrangeR(V15);
WidgetLED ledWhiteR(V16);



WidgetLED ledGreen(V17);
WidgetLED ledPrioLeft(V19);
WidgetLED ledPrioRight(V20);
*/

union mix_t
{
    std::uint32_t theDWord;
    std::uint8_t theBytes[4];
};

static bool bWifiConnected = false;
static bool bUDPConnected = false;

void ProcessUDPPacket (AsyncUDPPacket packet)
{
  /*Serial.print("UDP Packet Type: ");
  Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
  Serial.print(", From: ");
  Serial.print(packet.remoteIP());
  Serial.print(":");
  Serial.print(packet.remotePort());
  Serial.print(", To: ");
  Serial.print(packet.localIP());
  Serial.print(":");
  Serial.print(packet.localPort());
  Serial.print(", Length: ");
  Serial.print(packet.length());
  Serial.print(", Data: ");
  Serial.write(packet.data(), packet.length());
  Serial.print(packet.data()[0]);
  Serial.print(packet.data()[1]);
  Serial.print(packet.data()[2]);
  Serial.print(packet.data()[3]);
  Serial.println();*/
  //reply to the client
  //packet.printf("Got %u bytes of data", packet.length());
  mix_t Event;
  Event.theBytes[0] = packet.data()[0];
  Event.theBytes[1] = packet.data()[1];
  Event.theBytes[2] = packet.data()[2];
  Event.theBytes[3] = packet.data()[3];
  MyUDPIOHandler.InputChanged(Event.theDWord);
}

void UDPCheck()
{
static long NextTimeToCheckUDP = millis() + 2500;

  /*if(millis() < NextTimeToCheckUDP)
    return;*/
  NextTimeToCheckUDP = millis() + 2500;
#ifdef HOMENETWORK
  if(WiFi.status() == WL_CONNECTED)
  {
    if(!bWifiConnected)
    {
      bWifiConnected = true;
    }

    if(!bUDPConnected)
    {// Somehow we should call this only once. It will keep on trying for ever.

      if(Commandudp.listen(1234))
      {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        Commandudp.onPacket([](AsyncUDPPacket packet) {
          ProcessUDPPacket (packet);
        });
    }
    MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
      bUDPConnected = true;
    }

  }

#else
    //if(bWifiConnected)
    {
      if(!bUDPConnected)
      {// Somehow we should call this only once. It will keep on trying for ever.

        if(Commandudp.listen(1234))
        {
            Serial.print("UDP Listening on IP: ");
            Serial.println(WiFi.softAPIP());
            Commandudp.onPacket([](AsyncUDPPacket packet) {
              ProcessUDPPacket (packet);
            });
        }
        MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
        MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
        bUDPConnected = true;
    }

  }
#endif
}

void UDPIOHandler::ConnectToAP()
{
  if(!bUDPConnected)
  {// We should call this only once.
    bUDPConnected = true;
    if(Commandudp.listen(1234))
    {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.softAPIP());
        Commandudp.onPacket([](AsyncUDPPacket packet) {
          ProcessUDPPacket (packet);
        });
    }
    MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
    MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);
  }
}

void UDPIOHandler::run()
{
  UDPCheck();
  //if(UDP.connected())
    //UDP.run();

};

UDPIOHandler::UDPIOHandler()
{
    //ctor

}

void UDPIOHandler::Start()
{
  /*WiFi.hostname(espHostName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);*/
}

UDPIOHandler::~UDPIOHandler()
{
    //dtor
}

void UDPIOHandler::ProcessLightsChange(uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
/*  if(event_data & MASK_RED )
    ledRed.on();
  else
    ledRed.off();
  if(event_data & MASK_GREEN )
    ledGreen.on();
  else
    ledGreen.off();
  if(event_data & MASK_WHITE_L )
    ledWhiteL.on();
  else
    ledWhiteL.off();
  if(event_data & MASK_WHITE_R )
    ledWhiteR.on();
  else
    ledWhiteR.off();
  if(event_data & MASK_ORANGE_L )
    ledOrangeL.on();
  else
    ledOrangeL.off();
  if(event_data & MASK_ORANGE_R )
    ledOrangeR.on();
  else
        ledOrangeR.off();
        */
}


void UDPIOHandler::update (FencingStateMachine *subject, uint32_t eventtype)
{
  uint32_t event_data = eventtype & SUB_TYPE_MASK;
  uint32_t maineventtype = eventtype & MAIN_TYPE_MASK ;
  char chrono[16];
  char strRound[8];
  int newseconds;
  int currentRound, nrOfRounds;

  switch(maineventtype)
  {

    case EVENT_LIGHTS:
      ProcessLightsChange(eventtype);
    break;
    case EVENT_WEAPON:

    break;

    case EVENT_SCORE_LEFT:
      //UDP.virtualWrite(V3, eventtype & DATA_24BIT_MASK);
    break;

    case EVENT_SCORE_RIGHT:
      //UDP.virtualWrite(V4, eventtype & DATA_24BIT_MASK);
    break;

    case EVENT_TIMER_STATE:
      if(eventtype & DATA_24BIT_MASK)
      {
        //UDP.setProperty(V0,"color",UDP_GREEN);
        //UDP.virtualWrite(V1,1);
      }
      else
      {
        //UDP.setProperty(V0,"color",UDP_RED);
        //UDP.virtualWrite(V1,0);
      }
    break;
    case EVENT_TIMER:
    newseconds = event_data & (DATA_BYTE1_MASK |DATA_BYTE2_MASK);

      if(previous_seconds != newseconds)
      {
        subject->GetFormattedStringTime(chrono,2,0);
        //UDP.virtualWrite(V0, chrono);
        previous_seconds = newseconds;
      }

    break;

    case EVENT_ROUND:
      currentRound = event_data & DATA_BYTE0_MASK;
      nrOfRounds = (event_data & DATA_BYTE1_MASK)>>8;
      if(!currentRound || !nrOfRounds)
      {
        sprintf(strRound," ");

      }
      else
        sprintf(strRound,"%d/%d",currentRound,nrOfRounds);

      //UDP.virtualWrite(V11, strRound);

    break;



  }

}

/*

// V1 is the Virual pin connected to the push button: Start/Stop timer
// V0 is the Virtual pin connected to the time display
UDP_WRITE(V1)
{
  int iTimerStartStopButton = param.asInt(); // assigning incoming value from pin V1 to a variable
  if(iTimerStartStopButton)
  {
    MyUDPIOHandler.InputChanged(UI_INPUT_START_TIMER | EVENT_UI_INPUT);

  }
  else
  {
    MyUDPIOHandler.InputChanged(UI_INPUT_STOP_TIMER | EVENT_UI_INPUT);

  }

}

// V0 is the Virtual pin connected to Reset Button
UDP_WRITE(V2)
{
  int ButtonStatus = param.asInt();
  if(ButtonStatus)
    MyUDPIOHandler.InputChanged(UI_INPUT_RESET | EVENT_UI_INPUT);

}

// V5-V8 are the Virtual pin connected score up / dwn buttons
// We need
UDP_WRITE(V5)
{
  static long allowNext = 0;
  int ButtonStatus = param.asInt();
  if(millis() > allowNext)
  {
    allowNext = millis() + 250;
    if(ButtonStatus)
      MyUDPIOHandler.InputChanged(UI_INPUT_INCR_SCORE_LEFT | EVENT_UI_INPUT);
  }

}

UDP_WRITE(V6)
{

  static long allowNext = 0;
  int ButtonStatus = param.asInt();
  if(millis() {> allowNext)
  {
    allowNext = millis() + 250;
    if(ButtonStatus)
      MyUDPIOHandler.InputChanged(UI_INPUT_DECR_SCORE_LEFT | EVENT_UI_INPUT);
  }
}

UDP_WRITE(V7)
{

  static long allowNext = 0;
  int ButtonStatus = param.asInt();
  if(millis() > allowNext)
  {
    allowNext = millis() + 250;
    if(ButtonStatus)
      MyUDPIOHandler.InputChanged(UI_INPUT_INCR_SCORE_RIGHT | EVENT_UI_INPUT);
  }

}
UDP_WRITE(V8)
{

  static long allowNext = 0;
  int ButtonStatus = param.asInt();
  if(millis() > allowNext)
  {
    allowNext = millis() + 250;
    if(ButtonStatus)
      MyUDPIOHandler.InputChanged(UI_INPUT_DECR_SCORE_RIGHT | EVENT_UI_INPUT);
  }
}

// SET round button pressed
UDP_WRITE(V10)
{
  if(param.asInt())
  {
    //CurentRound = 1;
    //UDP.virtualWrite(V11, strRoundInfo);
    MyUDPIOHandler.InputChanged(UI_INPUT_ROUND | EVENT_UI_INPUT);
  }

}

/*
// Prio Button Pressed
UDP_WRITE(V18)
{
long TimeOfPress = millis();
 // assigning incoming value from pin V18 to a variable
  if(ButtonStatus)
  {
    if(iPrio)
    {
      iPrio = 0;
      ledPrioLeft.off();
      ledPrioRight.off();
    }
    else
    {
        if(TimeOfPress%2)
        {
          iPrio = 1;
        }
        else
        {
          iPrio = 2;

        }
        for(int i = 0; i < 7 ; i++)
        {
          ledPrioLeft.off();
          ledPrioRight.on();
          setGreen(true);
          setRed(false);
          pixels.show();
          UDPDelay(300);
          ledPrioLeft.on();
          ledPrioRight.off();
          setGrCommandudpeen(false);
          setReCommandudpd(true);ProcessLightsChange
          pixels.show();
          UDPDelay(300);
        }
        if(iPrio == 2)
        {
          ledPrioLeft.off();
          ledPrioRight.on();
          setGreen(true);
          setRed(false);
          pixels.show();
        }
    }
    UDPDelay(1750);
    setGreen(false);
    setRed(false);
    pixels.show();

  }

}
*/
